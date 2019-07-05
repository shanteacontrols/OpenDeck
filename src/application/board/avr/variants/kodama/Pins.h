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

#include <avr/io.h>

#define SR_DIN_DATA_PORT            PORTD
#define SR_DIN_DATA_PIN             3

#define SR_DIN_CLK_PORT             PORTD
#define SR_DIN_CLK_PIN              4

#define SR_DIN_LATCH_PORT           PORTD
#define SR_DIN_LATCH_PIN            5

#define SR_OUT_DATA_PORT            PORTD
#define SR_OUT_DATA_PIN             2

#define SR_OUT_CLK_PORT             PORTD
#define SR_OUT_CLK_PIN              0

#define SR_OUT_LATCH_PORT           PORTD
#define SR_OUT_LATCH_PIN            1

#define MUX_S0_PORT                 PORTB
#define MUX_S0_PIN                  3

#define MUX_S1_PORT                 PORTB
#define MUX_S1_PIN                  1

#define MUX_S2_PORT                 PORTE
#define MUX_S2_PIN                  6

#define MUX_S3_PORT                 PORTB
#define MUX_S3_PIN                  2

#define MUX_1_IN_PORT               PORTF
#define MUX_1_IN_PIN                7

#define MUX_2_IN_PORT               PORTF
#define MUX_2_IN_PIN                6

#define MUX_3_IN_PORT               PORTF
#define MUX_3_IN_PIN                5

#define MUX_4_IN_PORT               PORTF
#define MUX_4_IN_PIN                4

#define BTLDR_BUTTON_INDEX          0x03