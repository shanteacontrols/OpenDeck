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

#ifdef BOARD_A_MEGA

#pragma once

//uncomment if leds use reverse logic for setting on/off state
//#define LED_INVERT

#define MAX_NUMBER_OF_ANALOG        6
#define MAX_NUMBER_OF_BUTTONS       8
#define MAX_NUMBER_OF_LEDS          6
#define MAX_NUMBER_OF_RGB_LEDS      (MAX_NUMBER_OF_LEDS/3)
#define MAX_NUMBER_OF_ENCODERS      (MAX_NUMBER_OF_BUTTONS/2)

#endif