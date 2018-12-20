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

#include <avr/io.h>

namespace Board
{
    namespace detail
    {
        #define SR_DIN_DATA_PORT            PORTD
        #define SR_DIN_DATA_PIN             7

        #define SR_DIN_CLK_PORT             PORTD
        #define SR_DIN_CLK_PIN              6

        #define SR_DIN_LATCH_PORT           PORTD
        #define SR_DIN_LATCH_PIN            4

        #define MUX_S0_PORT                 PORTF
        #define MUX_S0_PIN                  6

        #define MUX_S1_PORT                 PORTF
        #define MUX_S1_PIN                  5

        #define MUX_S2_PORT                 PORTF
        #define MUX_S2_PIN                  7

        #define MUX_S3_PORT                 PORTF
        #define MUX_S3_PIN                  4

        #define MUX_1_IN_PORT               PORTF
        #define MUX_1_IN_PIN                1

        #define BTLDR_BUTTON_PORT           PORTF
        #define BTLDR_BUTTON_PIN            0
    }
}