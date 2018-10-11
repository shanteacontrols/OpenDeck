/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <avr/io.h>

#define DI_1_PORT               PORTB
#define DI_1_PIN                6

#define DI_2_PORT               PORTB
#define DI_2_PIN                5

#define DI_3_PORT               PORTB
#define DI_3_PIN                4

#define DI_4_PORT               PORTB
#define DI_4_PIN                3

#define DI_5_PORT               PORTB
#define DI_5_PIN                2

#define DI_6_PORT               PORTB
#define DI_6_PIN                1

#define DI_7_PORT               PORTB
#define DI_7_PIN                0

#define DI_8_PORT               PORTE
#define DI_8_PIN                7

#define DI_9_PORT               PORTE
#define DI_9_PIN                6

#define DI_10_PORT              PORTA
#define DI_10_PIN               0

#define DI_11_PORT              PORTA
#define DI_11_PIN               5

#define DI_12_PORT              PORTA
#define DI_12_PIN               1

#define DI_13_PORT              PORTA
#define DI_13_PIN               6

#define DI_14_PORT              PORTA
#define DI_14_PIN               2

#define DI_15_PORT              PORTA
#define DI_15_PIN               7

#define DI_16_PORT              PORTA
#define DI_16_PIN               3


#define DO_1_PORT               PORTB
#define DO_1_PIN                7

#define DO_2_PORT               PORTD
#define DO_2_PIN                4

#define DO_3_PORT               PORTD
#define DO_3_PIN                5

#define DO_4_PORT               PORTD
#define DO_4_PIN                6

#define DO_5_PORT               PORTD
#define DO_5_PIN                7

#define DO_6_PORT               PORTE
#define DO_6_PIN                0

#define DO_7_PORT               PORTE
#define DO_7_PIN                1

#define DO_8_PORT               PORTC
#define DO_8_PIN                0

#define DO_9_PORT               PORTC
#define DO_9_PIN                1

#define DO_10_PORT              PORTC
#define DO_10_PIN               2

#define DO_11_PORT              PORTC
#define DO_11_PIN               3

#define DO_12_PORT              PORTC
#define DO_12_PIN               4

#define DO_13_PORT              PORTC
#define DO_13_PIN               5

#define DO_14_PORT              PORTC
#define DO_14_PIN               6

#define DO_15_PORT              PORTC
#define DO_15_PIN               7

#define DO_16_PORT              PORTA
#define DO_16_PIN               4


#define AI_1_PORT               PORTF
#define AI_1_PIN                0

#define AI_2_PORT               PORTF
#define AI_2_PIN                1

#define AI_3_PORT               PORTF
#define AI_3_PIN                2

#define AI_4_PORT               PORTF
#define AI_4_PIN                3

#define AI_5_PORT               PORTF
#define AI_5_PIN                4

#define AI_6_PORT               PORTF
#define AI_6_PIN                5

#define AI_7_PORT               PORTF
#define AI_7_PIN                6

#define AI_8_PORT               PORTF
#define AI_8_PIN                7

//on teensy, there is only one led which is used as an output
//use this led as indicator only on startup
#define LED_IN_PORT             DO_4_PORT
#define LED_IN_PIN              DO_4_PIN

#define BTLDR_BUTTON_PORT       PORTE
#define BTLDR_BUTTON_PIN        4