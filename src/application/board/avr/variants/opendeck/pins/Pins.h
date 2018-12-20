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
        ///
        /// \brief Pin definitions.
        /// @{

        #define SR_DIN_DATA_PORT            PORTD
        #define SR_DIN_DATA_PIN             5

        #define SR_DIN_CLK_PORT             PORTD
        #define SR_DIN_CLK_PIN              0

        #define SR_DIN_LATCH_PORT           PORTD
        #define SR_DIN_LATCH_PIN            1

        #define DEC_DM_A0_PORT              PORTB
        #define DEC_DM_A0_PIN               4

        #define DEC_DM_A1_PORT              PORTD
        #define DEC_DM_A1_PIN               6

        #define DEC_DM_A2_PORT              PORTD
        #define DEC_DM_A2_PIN               4

        #define DEC_LM_A0_PORT              PORTB
        #define DEC_LM_A0_PIN               1

        #define DEC_LM_A1_PORT              PORTB
        #define DEC_LM_A1_PIN               2

        #define DEC_LM_A2_PORT              PORTB
        #define DEC_LM_A2_PIN               3

        #define LED_ROW_1_PORT              PORTB
        #define LED_ROW_1_PIN               7

        #define LED_ROW_2_PORT              PORTD
        #define LED_ROW_2_PIN               7

        #define LED_ROW_3_PORT              PORTB
        #define LED_ROW_3_PIN               5

        #define LED_ROW_4_PORT              PORTC
        #define LED_ROW_4_PIN               7

        #define LED_ROW_5_PORT              PORTC
        #define LED_ROW_5_PIN               6

        #define LED_ROW_6_PORT              PORTB
        #define LED_ROW_6_PIN               6

        #define MUX_S0_PORT                 PORTF
        #define MUX_S0_PIN                  5

        #define MUX_S1_PORT                 PORTF
        #define MUX_S1_PIN                  7

        #define MUX_S2_PORT                 PORTF
        #define MUX_S2_PIN                  4

        #define MUX_S3_PORT                 PORTF
        #define MUX_S3_PIN                  6

        #define MUX_1_IN_PORT               PORTF
        #define MUX_1_IN_PIN                0

        #define MUX_2_IN_PORT               PORTF
        #define MUX_2_IN_PIN                1

        #define LED_IN_PORT                 PORTE
        #define LED_IN_PIN                  6

        #define LED_OUT_PORT                PORTE
        #define LED_OUT_PIN                 2

        #define BTLDR_BUTTON_PORT           PORTB
        #define BTLDR_BUTTON_PIN            2

        /// @}
    }
}