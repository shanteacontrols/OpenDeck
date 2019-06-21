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
            const uint8_t adcChannelArray[MAX_NUMBER_OF_ANALOG] =
            {
                0,  //AI_1_PIN, ADC0
                1,  //AI_2_PIN, ADC1
                2,  //AI_3_PIN, ADC2
                3,  //AI_4_PIN, ADC3
                4,  //AI_5_PIN, ADC4
                5,  //AI_6_PIN, ADC5
                6,  //AI_7_PIN, ADC6
                7,  //AI_8_PIN, ADC7
                8,  //AI_9_PIN, ADC8
                9,  //AI_10_PIN, ADC9
                10, //AI_11_PIN, ADC10
                11, //AI_12_PIN, ADC11
                12, //AI_13_PIN, ADC12
                13, //AI_14_PIN, ADC13
                14, //AI_15_PIN, ADC14
                15, //AI_16_PIN, ADC15
            };

            ///
            /// \brief Array holding ports and bits for all digital input pins.
            ///
            const mcuPin_t dInPins[MAX_NUMBER_OF_BUTTONS] =
            {
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
                },

                {
                    .port = &DI_7_PORT,
                    .pin = DI_7_PIN,
                },

                {
                    .port = &DI_8_PORT,
                    .pin = DI_8_PIN,
                },

                {
                    .port = &DI_9_PORT,
                    .pin = DI_9_PIN,
                },

                {
                    .port = &DI_10_PORT,
                    .pin = DI_10_PIN,
                },

                {
                    .port = &DI_11_PORT,
                    .pin = DI_11_PIN,
                },

                {
                    .port = &DI_12_PORT,
                    .pin = DI_12_PIN,
                },

                {
                    .port = &DI_13_PORT,
                    .pin = DI_13_PIN,
                },

                {
                    .port = &DI_14_PORT,
                    .pin = DI_14_PIN,
                },

                {
                    .port = &DI_15_PORT,
                    .pin = DI_15_PIN,
                },

                {
                    .port = &DI_16_PORT,
                    .pin = DI_16_PIN,
                },

                {
                    .port = &DI_17_PORT,
                    .pin = DI_17_PIN,
                },

                {
                    .port = &DI_18_PORT,
                    .pin = DI_18_PIN,
                },

                {
                    .port = &DI_19_PORT,
                    .pin = DI_19_PIN,
                },

                {
                    .port = &DI_20_PORT,
                    .pin = DI_20_PIN,
                },

                {
                    .port = &DI_21_PORT,
                    .pin = DI_21_PIN,
                },

                {
                    .port = &DI_22_PORT,
                    .pin = DI_22_PIN,
                },

                {
                    .port = &DI_23_PORT,
                    .pin = DI_23_PIN,
                },

                {
                    .port = &DI_24_PORT,
                    .pin = DI_24_PIN,
                },

                {
                    .port = &DI_25_PORT,
                    .pin = DI_25_PIN,
                },

                {
                    .port = &DI_26_PORT,
                    .pin = DI_26_PIN,
                },

                {
                    .port = &DI_27_PORT,
                    .pin = DI_27_PIN,
                },

                {
                    .port = &DI_28_PORT,
                    .pin = DI_28_PIN,
                },

                {
                    .port = &DI_29_PORT,
                    .pin = DI_29_PIN,
                },

                {
                    .port = &DI_30_PORT,
                    .pin = DI_30_PIN,
                },

                {
                    .port = &DI_31_PORT,
                    .pin = DI_31_PIN,
                },

                {
                    .port = &DI_32_PORT,
                    .pin = DI_32_PIN,
                },
            };

            ///
            /// \brief Array holding ports and bits for all digital output pins.
            ///
            mcuPin_t dOutPins[MAX_NUMBER_OF_LEDS] =
            {
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
                },

                {
                    .port = &DO_7_PORT,
                    .pin = DO_7_PIN,
                },

                {
                    .port = &DO_8_PORT,
                    .pin = DO_8_PIN,
                },

                {
                    .port = &DO_9_PORT,
                    .pin = DO_9_PIN,
                },

                {
                    .port = &DO_10_PORT,
                    .pin = DO_10_PIN,
                },

                {
                    .port = &DO_11_PORT,
                    .pin = DO_11_PIN,
                },

                {
                    .port = &DO_12_PORT,
                    .pin = DO_12_PIN,
                },

                {
                    .port = &DO_13_PORT,
                    .pin = DO_13_PIN,
                },

                {
                    .port = &DO_14_PORT,
                    .pin = DO_14_PIN,
                },

                {
                    .port = &DO_15_PORT,
                    .pin = DO_15_PIN,
                },

                {
                    .port = &DO_16_PORT,
                    .pin = DO_16_PIN,
                }
            };
        }

        uint8_t adcChannel(uint8_t index)
        {
            return adcChannelArray[index];
        }

        Board::mcuPin_t button(uint8_t index)
        {
            return dInPins[index];
        }

        Board::mcuPin_t led(uint8_t index)
        {
            return dOutPins[index];
        }
    }
}