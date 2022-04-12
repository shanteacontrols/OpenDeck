/*

Copyright 2015-2022 Igor Petrovic

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

#ifdef ADC_SUPPORTED

#include "board/common/constants/IO.h"
#include "core/src/util/Util.h"
#include "board/Board.h"
#include "board/Internal.h"
#include <Target.h>

namespace
{
#ifndef NUMBER_OF_MUX
    constexpr size_t ANALOG_IN_BUFFER_SIZE = NR_OF_ANALOG_INPUTS;
#else
    constexpr size_t   ANALOG_IN_BUFFER_SIZE = (NUMBER_OF_MUX_INPUTS * NUMBER_OF_MUX);
#endif

    /// Used to indicate that the new reading has been made.
    constexpr uint16_t NEW_READING_FLAG = 0x8000;

#ifdef ADC_10_BIT
    constexpr uint16_t ADC_MAX_READING = 1023;
#elif defined(ADC_12_BIT)
    constexpr uint16_t ADC_MAX_READING       = 4095;
#else
#error Unsupported ADC resolution
#endif

    uint8_t           _analogIndex;
    volatile uint16_t _analogBuffer[ANALOG_IN_BUFFER_SIZE];

#ifdef NUMBER_OF_MUX
    uint8_t _activeMux;
    uint8_t _activeMuxInput;

    /// Configures one of 16 inputs/outputs on 4067 multiplexer.
    inline void setMuxInput()
    {
        core::util::BIT_READ(_activeMuxInput, 0) ? CORE_MCU_IO_SET_HIGH(MUX_PORT_S0, MUX_PIN_S0) : CORE_MCU_IO_SET_LOW(MUX_PORT_S0, MUX_PIN_S0);
        core::util::BIT_READ(_activeMuxInput, 1) ? CORE_MCU_IO_SET_HIGH(MUX_PORT_S1, MUX_PIN_S1) : CORE_MCU_IO_SET_LOW(MUX_PORT_S1, MUX_PIN_S1);
        core::util::BIT_READ(_activeMuxInput, 2) ? CORE_MCU_IO_SET_HIGH(MUX_PORT_S2, MUX_PIN_S2) : CORE_MCU_IO_SET_LOW(MUX_PORT_S2, MUX_PIN_S2);
#ifdef MUX_PORT_S3
        core::util::BIT_READ(_activeMuxInput, 3) ? CORE_MCU_IO_SET_HIGH(MUX_PORT_S3, MUX_PIN_S3) : CORE_MCU_IO_SET_LOW(MUX_PORT_S3, MUX_PIN_S3);
#endif
    }
#endif
}    // namespace

namespace Board::IO
{
    bool analogValue(size_t analogID, uint16_t& value)
    {
        if (analogID >= NR_OF_ANALOG_INPUTS)
        {
            return false;
        }

        analogID = detail::map::adcIndex(analogID);

        CORE_MCU_ATOMIC_SECTION
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
}    // namespace Board::IO

namespace Board::detail::IO
{
    void adcISR(uint16_t adcValue)
    {
        static bool firstReading = false;
        firstReading             = !firstReading;

        if (!firstReading && (adcValue <= ADC_MAX_READING))
        {
#ifdef NUMBER_OF_MUX
            detail::IO::dischargeMux();
#endif

            _analogBuffer[_analogIndex] = adcValue | NEW_READING_FLAG;
            _analogIndex++;
#ifdef NUMBER_OF_MUX
            _activeMuxInput++;

            bool switchMux = (_activeMuxInput == NUMBER_OF_MUX_INPUTS);

            if (switchMux)
#else
            if (_analogIndex == NR_OF_ANALOG_INPUTS)
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
                core::mcu::adc::setChannel(Board::detail::map::adcChannel(_activeMux));
#endif
            }

// always switch to next read pin
#ifdef NUMBER_OF_MUX
            setMuxInput();
#else
            core::mcu::adc::setChannel(Board::detail::map::adcChannel(_analogIndex));
#endif
        }

#ifdef NUMBER_OF_MUX
        detail::IO::restoreMux(_activeMux);
#endif

        core::mcu::adc::startItConversion();
    }
}    // namespace Board::detail::IO

#endif