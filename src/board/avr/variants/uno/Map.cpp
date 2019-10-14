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
#include "board/common/Map.h"

namespace Board
{
    namespace map
    {
        namespace
        {
            const uint8_t adcChannelArray[MAX_NUMBER_OF_ANALOG]{
                AI_1_PIN,
                AI_2_PIN,
                AI_3_PIN,
                AI_4_PIN,
                AI_5_PIN,
                AI_6_PIN
            };

            ///
            /// \brief Array holding ports and bits for all digital input pins.
            ///
            const core::CORE_ARCH::pins::mcuPin_t dInPins[MAX_NUMBER_OF_BUTTONS] = {
                {
                    .port = &DI_1_PORT,
                    .pin = DI_1_PIN,
                },

                {
                    .port = &DI_2_PORT,
                    .pin = DI_2_PIN,
                },

                {
                    .port = &DI_3_PORT,
                    .pin = DI_3_PIN,
                },

                {
                    .port = &DI_4_PORT,
                    .pin = DI_4_PIN,
                },

                {
                    .port = &DI_5_PORT,
                    .pin = DI_5_PIN,
                },

                {
                    .port = &DI_6_PORT,
                    .pin = DI_6_PIN,
                }
            };

            ///
            /// \brief Array holding ports and bits for all digital output pins.
            ///
            const core::CORE_ARCH::pins::mcuPin_t dOutPins[MAX_NUMBER_OF_LEDS] = {
                {
                    .port = &DO_1_PORT,
                    .pin = DO_1_PIN,
                },

                {
                    .port = &DO_2_PORT,
                    .pin = DO_2_PIN,
                },

                {
                    .port = &DO_3_PORT,
                    .pin = DO_3_PIN,
                },

                {
                    .port = &DO_4_PORT,
                    .pin = DO_4_PIN,
                },

                {
                    .port = &DO_5_PORT,
                    .pin = DO_5_PIN,
                },

                {
                    .port = &DO_6_PORT,
                    .pin = DO_6_PIN,
                }
            };
        }    // namespace

        uint8_t adcChannel(uint8_t index)
        {
            return adcChannelArray[index];
        }

        core::CORE_ARCH::pins::mcuPin_t button(uint8_t index)
        {
            return dInPins[index];
        }

        core::CORE_ARCH::pins::mcuPin_t led(uint8_t index)
        {
            return dOutPins[index];
        }
    }    // namespace map
}    // namespace Board