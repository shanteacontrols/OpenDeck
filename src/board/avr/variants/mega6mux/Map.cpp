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
            const uint8_t adcChannelArray[NUMBER_OF_MUX] = {
                0,    //MUX_1_IN_PIN,
                1,    //MUX_2_IN_PIN,
                2,    //MUX_3_IN_PIN,
                3,    //MUX_4_IN_PIN,
                4,    //MUX_5_IN_PIN,
                5,    //MUX_6_IN_PIN,
            };

            ///
            /// \brief Array holding ports and bits for all digital input pins.
            ///
            const core::io::mcuPin_t dInPins[MAX_NUMBER_OF_BUTTONS] = {
                {
                    .port = &DI_1_PORT,
                    .index = DI_1_PIN,
                },

                {
                    .port = &DI_2_PORT,
                    .index = DI_2_PIN,
                },

                {
                    .port = &DI_3_PORT,
                    .index = DI_3_PIN,
                },

                {
                    .port = &DI_4_PORT,
                    .index = DI_4_PIN,
                },

                {
                    .port = &DI_5_PORT,
                    .index = DI_5_PIN,
                },

                {
                    .port = &DI_6_PORT,
                    .index = DI_6_PIN,
                },

                {
                    .port = &DI_7_PORT,
                    .index = DI_7_PIN,
                },

                {
                    .port = &DI_8_PORT,
                    .index = DI_8_PIN,
                },

                {
                    .port = &DI_9_PORT,
                    .index = DI_9_PIN,
                },

                {
                    .port = &DI_10_PORT,
                    .index = DI_10_PIN,
                },

                {
                    .port = &DI_11_PORT,
                    .index = DI_11_PIN,
                },

                {
                    .port = &DI_12_PORT,
                    .index = DI_12_PIN,
                },

                {
                    .port = &DI_13_PORT,
                    .index = DI_13_PIN,
                },

                {
                    .port = &DI_14_PORT,
                    .index = DI_14_PIN,
                },

                {
                    .port = &DI_15_PORT,
                    .index = DI_15_PIN,
                },

                {
                    .port = &DI_16_PORT,
                    .index = DI_16_PIN,
                },

                {
                    .port = &DI_17_PORT,
                    .index = DI_17_PIN,
                },

                {
                    .port = &DI_18_PORT,
                    .index = DI_18_PIN,
                },

                {
                    .port = &DI_19_PORT,
                    .index = DI_19_PIN,
                },

                {
                    .port = &DI_20_PORT,
                    .index = DI_20_PIN,
                },

                {
                    .port = &DI_21_PORT,
                    .index = DI_21_PIN,
                },

                {
                    .port = &DI_22_PORT,
                    .index = DI_22_PIN,
                },

                {
                    .port = &DI_23_PORT,
                    .index = DI_23_PIN,
                },

                {
                    .port = &DI_24_PORT,
                    .index = DI_24_PIN,
                },

                {
                    .port = &DI_25_PORT,
                    .index = DI_25_PIN,
                },

                {
                    .port = &DI_26_PORT,
                    .index = DI_26_PIN,
                },

                {
                    .port = &DI_27_PORT,
                    .index = DI_27_PIN,
                },

                {
                    .port = &DI_28_PORT,
                    .index = DI_28_PIN,
                },

                {
                    .port = &DI_29_PORT,
                    .index = DI_29_PIN,
                },

                {
                    .port = &DI_30_PORT,
                    .index = DI_30_PIN,
                },
            };

            ///
            /// \brief Array holding ports and bits for all digital output pins.
            ///
            const core::io::mcuPin_t dOutPins[MAX_NUMBER_OF_LEDS] = {
                {
                    .port = &DO_1_PORT,
                    .index = DO_1_PIN,
                },

                {
                    .port = &DO_2_PORT,
                    .index = DO_2_PIN,
                },

                {
                    .port = &DO_3_PORT,
                    .index = DO_3_PIN,
                },

                {
                    .port = &DO_4_PORT,
                    .index = DO_4_PIN,
                },

                {
                    .port = &DO_5_PORT,
                    .index = DO_5_PIN,
                },

                {
                    .port = &DO_6_PORT,
                    .index = DO_6_PIN,
                },

                {
                    .port = &DO_7_PORT,
                    .index = DO_7_PIN,
                },

                {
                    .port = &DO_8_PORT,
                    .index = DO_8_PIN,
                },

                {
                    .port = &DO_9_PORT,
                    .index = DO_9_PIN,
                },

                {
                    .port = &DO_10_PORT,
                    .index = DO_10_PIN,
                },

                {
                    .port = &DO_11_PORT,
                    .index = DO_11_PIN,
                },

                {
                    .port = &DO_12_PORT,
                    .index = DO_12_PIN,
                },

                {
                    .port = &DO_13_PORT,
                    .index = DO_13_PIN,
                },

                {
                    .port = &DO_14_PORT,
                    .index = DO_14_PIN,
                },

                {
                    .port = &DO_15_PORT,
                    .index = DO_15_PIN,
                },

                {
                    .port = &DO_16_PORT,
                    .index = DO_16_PIN,
                },

                {
                    .port = &DO_17_PORT,
                    .index = DO_17_PIN,
                },

                {
                    .port = &DO_18_PORT,
                    .index = DO_18_PIN,
                },

                {
                    .port = &DO_19_PORT,
                    .index = DO_19_PIN,
                },

                {
                    .port = &DO_20_PORT,
                    .index = DO_20_PIN,
                },

                {
                    .port = &DO_21_PORT,
                    .index = DO_21_PIN,
                },

                {
                    .port = &DO_22_PORT,
                    .index = DO_22_PIN,
                },

                {
                    .port = &DO_23_PORT,
                    .index = DO_23_PIN,
                },

                {
                    .port = &DO_24_PORT,
                    .index = DO_24_PIN,
                }
            };
        }    // namespace

        uint8_t adcChannel(uint8_t index)
        {
            return adcChannelArray[index];
        }

        uint8_t muxChannel(uint8_t index)
        {
            return index;
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
}    // namespace Board