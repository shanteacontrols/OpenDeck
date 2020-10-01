/*

Copyright 2015-2020 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "board/common/constants/IO.h"
#include "core/src/general/ADC.h"
#include "core/src/general/Helpers.h"
#include "core/src/general/Atomic.h"
#include "board/Board.h"
#include "board/Internal.h"
#include "Pins.h"

///
/// \brief Used to indicate that the new reading has been made.
///
#define NEW_READING_FLAG 0x8000

namespace
{
    uint8_t           analogIndex;
    volatile uint16_t analogBuffer[MAX_NUMBER_OF_ANALOG];

#ifdef NUMBER_OF_MUX
    uint8_t activeMux;
    uint8_t activeMuxInput;

    ///
    /// \brief Configures one of 16 inputs/outputs on 4067 multiplexer.
    ///
    inline void setMuxInput()
    {
        BIT_READ(activeMuxInput, 0) ? CORE_IO_SET_HIGH(MUX_PORT_S0, MUX_PIN_S0) : CORE_IO_SET_LOW(MUX_PORT_S0, MUX_PIN_S0);
        BIT_READ(activeMuxInput, 1) ? CORE_IO_SET_HIGH(MUX_PORT_S1, MUX_PIN_S1) : CORE_IO_SET_LOW(MUX_PORT_S1, MUX_PIN_S1);
        BIT_READ(activeMuxInput, 2) ? CORE_IO_SET_HIGH(MUX_PORT_S2, MUX_PIN_S2) : CORE_IO_SET_LOW(MUX_PORT_S2, MUX_PIN_S2);
#ifdef MUX_PORT_S3
        BIT_READ(activeMuxInput, 3) ? CORE_IO_SET_HIGH(MUX_PORT_S3, MUX_PIN_S3) : CORE_IO_SET_LOW(MUX_PORT_S3, MUX_PIN_S3);
#endif
    }
#endif
}    // namespace

namespace Board
{
    namespace io
    {
        bool analogValue(uint8_t analogID, uint16_t& value)
        {
            if (analogID >= MAX_NUMBER_OF_ANALOG)
                return false;

            analogID = detail::map::adcIndex(analogID);

            ATOMIC_SECTION
            {
                value = analogBuffer[analogID];
                analogBuffer[analogID] &= ~NEW_READING_FLAG;
            }

            if (value & NEW_READING_FLAG)
            {
                value &= ~NEW_READING_FLAG;
                return true;
            }

            return false;
        }
    }    // namespace io

    namespace detail
    {
        namespace isrHandling
        {
            void adc(uint16_t adcValue)
            {
                static bool firstReading = false;

                firstReading = !firstReading;

                if (!firstReading)
                {
#ifdef NUMBER_OF_MUX
                    detail::io::dischargeMux();
#endif

                    analogBuffer[analogIndex] = adcValue;
                    analogBuffer[analogIndex] |= NEW_READING_FLAG;
                    analogIndex++;
#ifdef NUMBER_OF_MUX
                    activeMuxInput++;

                    bool switchMux = (activeMuxInput == NUMBER_OF_MUX_INPUTS);

                    if (switchMux)
#else
                    if (analogIndex == MAX_NUMBER_OF_ANALOG)
#endif
                    {
#ifdef NUMBER_OF_MUX
                        activeMuxInput = 0;
                        activeMux++;

                        if (activeMux == NUMBER_OF_MUX)
                        {
                            activeMux = 0;
#endif
                            analogIndex = 0;
#ifdef NUMBER_OF_MUX
                        }
#endif

#ifdef NUMBER_OF_MUX
                        //switch to next mux once all mux inputs are read
                        core::adc::setChannel(Board::detail::map::adcChannel(activeMux));
#endif
                    }

//always switch to next read pin
#ifdef NUMBER_OF_MUX
                    setMuxInput();
#else
                    core::adc::setChannel(Board::detail::map::adcChannel(analogIndex));
#endif
                }

#ifdef NUMBER_OF_MUX
                detail::io::restoreMux(activeMux);
#endif

                core::adc::startConversion();
            }
        }    // namespace isrHandling
    }        // namespace detail
}    // namespace Board