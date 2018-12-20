/*

Copyright 2015-2018 Igor Petrovic

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

#pragma once

#include "board/avr/DataTypes.h"
#include "Pins.h"

namespace Board
{
    namespace detail
    {
        const uint8_t adcChannelArray[MAX_NUMBER_OF_ANALOG] =
        {
            AI_1_PIN,
            AI_2_PIN,
            AI_3_PIN,
            AI_4_PIN,
            AI_5_PIN,
            AI_6_PIN,
            AI_7_PIN,
            AI_8_PIN
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
            }
        };

        ///
        /// \brief Array holding ports and bits for all digital output pins.
        ///
        const mcuPin_t dOutPins[MAX_NUMBER_OF_LEDS] =
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
}