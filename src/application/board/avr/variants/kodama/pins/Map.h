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

#include "Pins.h"

namespace Board
{
    namespace detail
    {
        const uint8_t adcChannelArray[NUMBER_OF_MUX] =
        {
            5,  //MUX_1_IN_PIN,
            6,  //MUX_2_IN_PIN,
            7,  //MUX_3_IN_PIN,
            13, //MUX_4_IN_PIN,
        };

        //12 connected leds on kodama board
        const uint8_t ledMapArray[12] =
        {
            0,
            14,
            2,
            1,
            3,
            4,
            9,
            8,
            11,
            10,
            13,
            12
        };
    }
}