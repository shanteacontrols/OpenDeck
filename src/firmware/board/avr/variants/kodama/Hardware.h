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

#define BUTTON_DEBOUNCE_COMPARE         0b11111100

//uncomment if leds use reverse logic for setting on/off state
// #define LED_INVERT

#define NUMBER_OF_MUX                   4
#define NUMBER_OF_MUX_INPUTS            16

#define NUMBER_OF_IN_SR                 2
#define NUMBER_OF_IN_SR_INPUTS          8

#define NUMBER_OF_OUT_SR                2
#define NUMBER_OF_OUT_SR_INPUTS         8

#define MAX_NUMBER_OF_ANALOG            (NUMBER_OF_MUX*NUMBER_OF_MUX_INPUTS)
#define MAX_NUMBER_OF_BUTTONS           (NUMBER_OF_IN_SR*NUMBER_OF_IN_SR_INPUTS)
#define MAX_NUMBER_OF_LEDS              (NUMBER_OF_OUT_SR*NUMBER_OF_OUT_SR_INPUTS)
#define MAX_NUMBER_OF_RGB_LEDS          (MAX_NUMBER_OF_LEDS/3)
#define MAX_NUMBER_OF_ENCODERS          (MAX_NUMBER_OF_BUTTONS/2)
