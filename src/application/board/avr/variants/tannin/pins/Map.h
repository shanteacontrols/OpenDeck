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

#pragma once

#include "board/avr/DataTypes.h"
#include "Pins.h"

namespace Board
{
    namespace detail
    {
        ///
        /// \brief Array holding ports and bits for all LED rows.
        ///
        const mcuPin_t ledRowPins[NUMBER_OF_LED_ROWS] =
        {
            {
                .port = &LED_ROW_1_PORT,
                .pin = LED_ROW_1_PIN,
            },

            {
                .port = &LED_ROW_2_PORT,
                .pin = LED_ROW_2_PIN,
            },

            {
                .port = &LED_ROW_3_PORT,
                .pin = LED_ROW_3_PIN,
            },

            {
                .port = &LED_ROW_4_PORT,
                .pin = LED_ROW_4_PIN,
            }
        };

        ///
        /// \brief Array holding ADC read pins/channels.
        ///
        const uint8_t adcChannelArray[NUMBER_OF_MUX] =
        {
            MUX_1_IN_PIN,
        };
    }
}