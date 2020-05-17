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
                uint8_t aInChannels[MAX_NUMBER_OF_ANALOG] = {
                    0,
                    1,
                    4,
                    5,
                    6,
                    7,
                    12,
                    13
                };

                ///
                /// \brief Array holding ports and bits for all digital input pins.
                ///
                const core::io::mcuPin_t dInPins[MAX_NUMBER_OF_BUTTONS] = {
                    CORE_IO_MCU_PIN_DEF(DI_1_PORT, DI_1_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_2_PORT, DI_2_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_3_PORT, DI_3_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_4_PORT, DI_4_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_5_PORT, DI_5_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_6_PORT, DI_6_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_7_PORT, DI_7_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_8_PORT, DI_8_PIN),
                };
            }    // namespace

            uint32_t adcChannel(uint8_t index)
            {
                return aInChannels[index];
            }

            core::io::mcuPin_t button(uint8_t index)
            {
                return dInPins[index];
            }
        }    // namespace map
    }        // namespace detail
}    // namespace Board