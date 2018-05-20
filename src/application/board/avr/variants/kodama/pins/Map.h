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

#include "Pins.h"

const uint8_t muxInPinArray[] =
{
    5,  //MUX_1_IN_PIN,
    6,  //MUX_2_IN_PIN,
    7,  //MUX_3_IN_PIN,
    13, //MUX_4_IN_PIN,
};

//12 leds on kodama board
const uint8_t ledMapArray[12] =
{
    0,
    14,
    2,
    1,
    3,
    4,
    9,
    8,
    11,
    10,
    13,
    12
};