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

#define DI_1_PORT           PORTD
#define DI_1_PIN            1

#define DI_2_PORT           PORTD
#define DI_2_PIN            0

#define DI_3_PORT           PORTD
#define DI_3_PIN            4

#define DI_4_PORT           PORTC
#define DI_4_PIN            6

#define DI_5_PORT           PORTD
#define DI_5_PIN            7

#define DI_6_PORT           PORTE
#define DI_6_PIN            6


#define DO_1_PORT           PORTB
#define DO_1_PIN            4

#define DO_2_PORT           PORTB
#define DO_2_PIN            5

#define DO_3_PORT           PORTB
#define DO_3_PIN            6

#define DO_4_PORT           PORTB
#define DO_4_PIN            2

#define DO_5_PORT           PORTB
#define DO_5_PIN            3

#define DO_6_PORT           PORTB
#define DO_6_PIN            1


#define AI_1_PORT           PORTF
#define AI_1_PIN            7

#define AI_2_PORT           PORTF
#define AI_2_PIN            6

#define AI_3_PORT           PORTF
#define AI_3_PIN            5

#define AI_4_PORT           PORTF
#define AI_4_PIN            4


#define LED_OUT_PORT        PORTD
#define LED_OUT_PIN         5

#define LED_IN_PORT         PORTB
#define LED_IN_PIN          0

#define BTLDR_BUTTON_PORT   PORTB
#define BTLDR_BUTTON_PIN    2