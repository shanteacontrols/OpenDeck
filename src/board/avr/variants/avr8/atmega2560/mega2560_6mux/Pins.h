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

#define DI_1_PORT           PORTB
#define DI_1_PIN            1

#define DI_2_PORT           PORTB
#define DI_2_PIN            3

#define DI_3_PORT           PORTL
#define DI_3_PIN            1

#define DI_4_PORT           PORTL
#define DI_4_PIN            3

#define DI_5_PORT           PORTL
#define DI_5_PIN            5

#define DI_6_PORT           PORTL
#define DI_6_PIN            7

#define DI_7_PORT           PORTG
#define DI_7_PIN            1

#define DI_8_PORT           PORTD
#define DI_8_PIN            7

#define DI_9_PORT           PORTC
#define DI_9_PIN            1

#define DI_10_PORT          PORTC
#define DI_10_PIN           3

#define DI_11_PORT          PORTC
#define DI_11_PIN           5

#define DI_12_PORT          PORTC
#define DI_12_PIN           7

#define DI_13_PORT          PORTA
#define DI_13_PIN           6

#define DI_14_PORT          PORTA
#define DI_14_PIN           4

#define DI_15_PORT          PORTA
#define DI_15_PIN           2

#define DI_16_PORT          PORTA
#define DI_16_PIN           0

#define DI_17_PORT          PORTB
#define DI_17_PIN           0

#define DI_18_PORT          PORTB
#define DI_18_PIN           2

#define DI_19_PORT          PORTL
#define DI_19_PIN           0

#define DI_20_PORT          PORTL
#define DI_20_PIN           2

#define DI_21_PORT          PORTL
#define DI_21_PIN           4

#define DI_22_PORT          PORTL
#define DI_22_PIN           6

#define DI_23_PORT          PORTG
#define DI_23_PIN           0

#define DI_24_PORT          PORTG
#define DI_24_PIN           2

#define DI_25_PORT          PORTC
#define DI_25_PIN           0

#define DI_26_PORT          PORTC
#define DI_26_PIN           2

#define DI_27_PORT          PORTC
#define DI_27_PIN           4

#define DI_28_PORT          PORTC
#define DI_28_PIN           6

#define DI_29_PORT          PORTF
#define DI_29_PIN           6

#define DI_30_PORT          PORTF
#define DI_30_PIN           7


#define MUX_S0_PORT         PORTA
#define MUX_S0_PIN          1

#define MUX_S1_PORT         PORTA
#define MUX_S1_PIN          3

#define MUX_S2_PORT         PORTA
#define MUX_S2_PIN          5

#define MUX_S3_PORT         PORTA
#define MUX_S3_PIN          7


#define DO_1_PORT           PORTH
#define DO_1_PIN            0

#define DO_2_PORT           PORTH
#define DO_2_PIN            1

#define DO_3_PORT           PORTJ
#define DO_3_PIN            0

#define DO_4_PORT           PORTJ
#define DO_4_PIN            1

#define DO_5_PORT           PORTE
#define DO_5_PIN            4

#define DO_6_PORT           PORTE
#define DO_6_PIN            5

#define DO_7_PORT           PORTG
#define DO_7_PIN            5

#define DO_8_PORT           PORTE
#define DO_8_PIN            3

#define DO_9_PORT           PORTH
#define DO_9_PIN            3

#define DO_10_PORT          PORTH
#define DO_10_PIN           4

#define DO_11_PORT          PORTH
#define DO_11_PIN           5

#define DO_12_PORT          PORTH
#define DO_12_PIN           6

#define DO_13_PORT          PORTB
#define DO_13_PIN           4

#define DO_14_PORT          PORTB
#define DO_14_PIN           5

#define DO_15_PORT          PORTB
#define DO_15_PIN           6

#define DO_16_PORT          PORTB
#define DO_16_PIN           7

#define DO_17_PORT          PORTK
#define DO_17_PIN           0

#define DO_18_PORT          PORTK
#define DO_18_PIN           1

#define DO_19_PORT          PORTK
#define DO_19_PIN           2

#define DO_20_PORT          PORTK
#define DO_20_PIN           3

#define DO_21_PORT          PORTK
#define DO_21_PIN           4

#define DO_22_PORT          PORTK
#define DO_22_PIN           5

#define DO_23_PORT          PORTK
#define DO_23_PIN           6

#define DO_24_PORT          PORTK
#define DO_24_PIN           7


#define MUX_1_IN_PORT       PORTF
#define MUX_1_IN_PIN        0

#define MUX_2_IN_PORT       PORTF
#define MUX_2_IN_PIN        1

#define MUX_3_IN_PORT       PORTF
#define MUX_3_IN_PIN        2

#define MUX_4_IN_PORT       PORTF
#define MUX_4_IN_PIN        3

#define MUX_5_IN_PORT       PORTF
#define MUX_5_IN_PIN        4

#define MUX_6_IN_PORT       PORTF
#define MUX_6_IN_PIN        5


#define BTLDR_BUTTON_PORT   PORTB
#define BTLDR_BUTTON_PIN    2