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

#pragma once

#include <avr/io.h>

#define SR_DIN_DATA_PORT            PORTG
#define SR_DIN_DATA_PIN             2

#define SR_DIN_CLK_PORT             PORTG
#define SR_DIN_CLK_PIN              1

#define SR_DIN_LATCH_PORT           PORTG
#define SR_DIN_LATCH_PIN            0


#define DEC_DM_A0_PORT              PORTL
#define DEC_DM_A0_PIN               2

#define DEC_DM_A1_PORT              PORTL
#define DEC_DM_A1_PIN               1

#define DEC_DM_A2_PORT              PORTL
#define DEC_DM_A2_PIN               0


#define DEC_LM_A0_PORT              PORTL
#define DEC_LM_A0_PIN               5

#define DEC_LM_A1_PORT              PORTL
#define DEC_LM_A1_PIN               4

#define DEC_LM_A2_PORT              PORTL
#define DEC_LM_A2_PIN               3


#define LED_ROW_1_PORT              PORTH
#define LED_ROW_1_PIN               3

#define LED_ROW_2_PORT              PORTH
#define LED_ROW_2_PIN               4

#define LED_ROW_3_PORT              PORTH
#define LED_ROW_3_PIN               5

#define LED_ROW_4_PORT              PORTE
#define LED_ROW_4_PIN               3

#define LED_ROW_5_PORT              PORTE
#define LED_ROW_5_PIN               4

#define LED_ROW_6_PORT              PORTE
#define LED_ROW_6_PIN               5


#define MUX_S0_PORT                 PORTB
#define MUX_S0_PIN                  3

#define MUX_S1_PORT                 PORTB
#define MUX_S1_PIN                  2

#define MUX_S2_PORT                 PORTB
#define MUX_S2_PIN                  1

#define MUX_S3_PORT                 PORTB
#define MUX_S3_PIN                  0


#define MUX_1_IN_PORT               PORTF
#define MUX_1_IN_PIN                0

#define MUX_2_IN_PORT               PORTF
#define MUX_2_IN_PIN                1

#define MUX_3_IN_PORT               PORTF
#define MUX_3_IN_PIN                2

#define MUX_4_IN_PORT               PORTF
#define MUX_4_IN_PIN                3

#define MUX_5_IN_PORT               PORTF
#define MUX_5_IN_PIN                4

#define MUX_6_IN_PORT               PORTF
#define MUX_6_IN_PIN                5

#define MUX_7_IN_PORT               PORTF
#define MUX_7_IN_PIN                6


#define BTLDR_BUTTON_PORT           PORTB
#define BTLDR_BUTTON_PIN            4

#define LED_BTLDR_PORT              PORTB
#define LED_BTLDR_PIN               7