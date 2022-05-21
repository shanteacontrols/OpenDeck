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
#ifdef ANALOG_INPUT_DRIVER_MULTIPLEXER

#include "board/common/constants/IO.h"
#include "core/src/util/Util.h"
#include "board/Board.h"
#include "board/Internal.h"
#include <Target.h>

namespace
{
    constexpr size_t ANALOG_IN_BUFFER_SIZE = (NUMBER_OF_MUX_INPUTS * NUMBER_OF_MUX);

    uint8_t           _analogIndex;
    volatile uint16_t _analogBuffer[ANALOG_IN_BUFFER_SIZE];
    uint8_t           _activeMux;
    uint8_t           _activeMuxInput;

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
}    // namespace

namespace Board::detail::IO
{
    void adcISR(uint16_t adcValue)
    {
        static bool firstReading = false;
        firstReading             = !firstReading;

        if (!firstReading && (adcValue <= core::mcu::adc::MAX))
        {
            detail::IO::dischargeMux();

            _analogBuffer[_analogIndex] = adcValue | Board::detail::IO::ADC_NEW_READING_FLAG;
            _analogIndex++;
            _activeMuxInput++;

            bool switchMux = (_activeMuxInput == NUMBER_OF_MUX_INPUTS);

            if (switchMux)
            {
                _activeMuxInput = 0;
                _activeMux++;

                if (_activeMux == NUMBER_OF_MUX)
                {
                    _activeMux   = 0;
                    _analogIndex = 0;
                }

                // switch to next mux once all mux inputs are read
                core::mcu::adc::setChannel(Board::detail::map::adcChannel(_activeMux));
            }

            // always switch to next read pin
            setMuxInput();
        }

        detail::IO::restoreMux(_activeMux);

        core::mcu::adc::startItConversion();
    }
}    // namespace Board::detail::IO

#include "Common.cpp.include"

#endif
#endif