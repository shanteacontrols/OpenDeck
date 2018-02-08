/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2017 Igor Petrovic

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

//uncomment if leds use reverse logic for setting on/off state
#define LED_INVERT

#define NUMBER_OF_MUX                   2
#define NUMBER_OF_MUX_INPUTS            16

#define NUMBER_OF_LED_COLUMNS           8
#define NUMBER_OF_LED_ROWS              6

#define NUMBER_OF_BUTTON_COLUMNS        8
#define NUMBER_OF_BUTTON_ROWS           8

#define MAX_NUMBER_OF_ANALOG            (NUMBER_OF_MUX*NUMBER_OF_MUX_INPUTS)
#define MAX_NUMBER_OF_BUTTONS           (NUMBER_OF_BUTTON_COLUMNS*NUMBER_OF_BUTTON_ROWS)
#define MAX_NUMBER_OF_LEDS              (NUMBER_OF_LED_COLUMNS*NUMBER_OF_LED_ROWS)
#define MAX_NUMBER_OF_RGB_LEDS          (MAX_NUMBER_OF_LEDS/3)
#define MAX_NUMBER_OF_ENCODERS          (MAX_NUMBER_OF_BUTTONS/2)

//active high logic
#define BTLDR_LED_ON(port, pin)         ((port) |= (1 << (pin)))
#define MIDI_LED_ON(port, pin)          BTLDR_LED_ON(port, pin)

#define BTLDR_LED_OFF(port, pin)        ((port) &= ~(1 << (pin)))
#define MIDI_LED_OFF(port, pin)         BTLDR_LED_OFF(port, pin)
