/*

Copyright 2015-2021 Igor Petrovic

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
#include <Pins.h>

#ifndef NUMBER_OF_MUX
#define ANALOG_IN_BUFFER_SIZE MAX_NUMBER_OF_ANALOG
#else
#define ANALOG_IN_BUFFER_SIZE (NUMBER_OF_MUX_INPUTS * NUMBER_OF_MUX)
#endif

/// Used to indicate that the new reading has been made.
#define NEW_READING_FLAG 0x8000

namespace
{
    uint8_t           _analogIndex;
    volatile uint16_t _analogBuffer[ANALOG_IN_BUFFER_SIZE];

#ifdef NUMBER_OF_MUX
    uint8_t _activeMux;
    uint8_t _activeMuxInput;

    /// Configures one of 16 inputs/outputs on 4067 multiplexer.
    inline void setMuxInput()
    {
        BIT_READ(_activeMuxInput, 0) ? CORE_IO_SET_HIGH(MUX_PORT_S0, MUX_PIN_S0) : CORE_IO_SET_LOW(MUX_PORT_S0, MUX_PIN_S0);
        BIT_READ(_activeMuxInput, 1) ? CORE_IO_SET_HIGH(MUX_PORT_S1, MUX_PIN_S1) : CORE_IO_SET_LOW(MUX_PORT_S1, MUX_PIN_S1);
        BIT_READ(_activeMuxInput, 2) ? CORE_IO_SET_HIGH(MUX_PORT_S2, MUX_PIN_S2) : CORE_IO_SET_LOW(MUX_PORT_S2, MUX_PIN_S2);
#ifdef MUX_PORT_S3
        BIT_READ(_activeMuxInput, 3) ? CORE_IO_SET_HIGH(MUX_PORT_S3, MUX_PIN_S3) : CORE_IO_SET_LOW(MUX_PORT_S3, MUX_PIN_S3);
#endif
    }
#endif
}    // namespace

namespace Board
{
    namespace io
    {
        bool analogValue(size_t analogID, uint16_t& value)
        {
            if (analogID >= MAX_NUMBER_OF_ANALOG)
                return false;

            analogID = detail::map::adcIndex(analogID);

            ATOMIC_SECTION
            {
                value = _analogBuffer[analogID];
                _analogBuffer[analogID] &= ~NEW_READING_FLAG;
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

                    _analogBuffer[_analogIndex] = adcValue;
                    _analogBuffer[_analogIndex] |= NEW_READING_FLAG;
                    _analogIndex++;
#ifdef NUMBER_OF_MUX
                    _activeMuxInput++;

                    bool switchMux = (_activeMuxInput == NUMBER_OF_MUX_INPUTS);

                    if (switchMux)
#else
                    if (_analogIndex == MAX_NUMBER_OF_ANALOG)
#endif
                    {
#ifdef NUMBER_OF_MUX
                        _activeMuxInput = 0;
                        _activeMux++;

                        if (_activeMux == NUMBER_OF_MUX)
                        {
                            _activeMux = 0;
#endif
                            _analogIndex = 0;
#ifdef NUMBER_OF_MUX
                        }
#endif

#ifdef NUMBER_OF_MUX
                        // switch to next mux once all mux inputs are read
                        core::adc::setChannel(Board::detail::map::adcChannel(_activeMux));
#endif
                    }

// always switch to next read pin
#ifdef NUMBER_OF_MUX
                    setMuxInput();
#else
                    core::adc::setChannel(Board::detail::map::adcChannel(_analogIndex));
#endif
                }

#ifdef NUMBER_OF_MUX
                detail::io::restoreMux(_activeMux);
#endif

                core::adc::startConversion();
            }
        }    // namespace isrHandling
    }        // namespace detail
}    // namespace Board