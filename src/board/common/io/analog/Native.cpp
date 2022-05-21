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
#ifdef ANALOG_INPUT_DRIVER_NATIVE

#include "board/common/constants/IO.h"
#include "core/src/util/Util.h"
#include "board/Board.h"
#include "board/Internal.h"
#include <Target.h>

namespace
{
    constexpr size_t ANALOG_IN_BUFFER_SIZE = NR_OF_ANALOG_INPUTS;

    uint8_t           _analogIndex;
    volatile uint16_t _analogBuffer[ANALOG_IN_BUFFER_SIZE];
}    // namespace

namespace Board::detail::IO
{
    void adcISR(uint16_t adcValue)
    {
        static bool firstReading = false;
        firstReading             = !firstReading;

        if (!firstReading && (adcValue <= core::mcu::adc::MAX))
        {
            _analogBuffer[_analogIndex] = adcValue | Board::detail::IO::ADC_NEW_READING_FLAG;
            _analogIndex++;
            if (_analogIndex == NR_OF_ANALOG_INPUTS)
            {
                _analogIndex = 0;
            }

            // always switch to next read pin
            core::mcu::adc::setChannel(Board::detail::map::adcChannel(_analogIndex));
        }

        core::mcu::adc::startItConversion();
    }
}    // namespace Board::detail::IO

#include "Common.cpp.include"

#endif
#endif