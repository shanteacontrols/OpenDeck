/*

Copyright 2015-2019 Igor Petrovic

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

#include "Pins.h"
#include "board/Internal.h"

namespace Board
{
    namespace detail
    {
        namespace map
        {
            namespace
            {
                const core::io::mcuPin_t aInPins[NUMBER_OF_MUX] = {
                    //port not used on avr mcus for adc channels, use any port
                    //adc channel doesn't have to match with adc pin
                    CORE_IO_MCU_PIN_DEF(PORTF, 6),
                    CORE_IO_MCU_PIN_DEF(PORTF, 5),
                    CORE_IO_MCU_PIN_DEF(PORTF, 4)
                };
            }

            core::io::mcuPin_t adcChannel(uint8_t index)
            {
                return aInPins[index];
            }

            uint8_t muxChannel(uint8_t index)
            {
                return index;
            }
        }    // namespace map
    }        // namespace detail
}    // namespace Board