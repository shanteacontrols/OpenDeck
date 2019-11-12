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
                const core::io::mcuPin_t aInPins[MAX_NUMBER_OF_ANALOG] = {
                    //port not used on avr mcus for adc channels, use any port
                    //adc channel doesn't have to match with adc pin
                    CORE_IO_MCU_PIN_DEF(AI_1_PORT, AI_1_PIN),
                    CORE_IO_MCU_PIN_DEF(AI_2_PORT, AI_2_PIN),
                    CORE_IO_MCU_PIN_DEF(AI_3_PORT, AI_3_PIN),
                    CORE_IO_MCU_PIN_DEF(AI_4_PORT, AI_4_PIN),
                    CORE_IO_MCU_PIN_DEF(AI_5_PORT, AI_5_PIN),
                    CORE_IO_MCU_PIN_DEF(AI_6_PORT, AI_6_PIN)
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
                    CORE_IO_MCU_PIN_DEF(DI_8_PORT, DI_8_PIN)
                };

                ///
                /// \brief Array holding ports and bits for all digital output pins.
                ///
                const core::io::mcuPin_t dOutPins[MAX_NUMBER_OF_LEDS] = {
                    CORE_IO_MCU_PIN_DEF(DO_1_PORT, DO_1_PIN),
                    CORE_IO_MCU_PIN_DEF(DO_2_PORT, DO_2_PIN),
                    CORE_IO_MCU_PIN_DEF(DO_3_PORT, DO_3_PIN),
                    CORE_IO_MCU_PIN_DEF(DO_4_PORT, DO_4_PIN),
                    CORE_IO_MCU_PIN_DEF(DO_5_PORT, DO_5_PIN),
                    CORE_IO_MCU_PIN_DEF(DO_6_PORT, DO_6_PIN)
                };
            }    // namespace

            core::io::mcuPin_t adcChannel(uint8_t index)
            {
                return aInPins[index];
            }

            core::io::mcuPin_t button(uint8_t index)
            {
                return dInPins[index];
            }

            core::io::mcuPin_t led(uint8_t index)
            {
                return dOutPins[index];
            }
        }    // namespace map
    }        // namespace detail
}    // namespace Board