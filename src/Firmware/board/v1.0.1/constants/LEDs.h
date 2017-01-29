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

#define NUMBER_OF_LED_TRANSITIONS           64
#define LED_ACTIVE_BIT                      0x00
#define LED_CONSTANT_ON_BIT                 0x01
#define LED_BLINK_ON_BIT                    0x02
#define LED_BLINK_STATE_BIT                 0x03
#define LED_RGB_BIT                         0x04
#define LED_RGB_R_BIT                       0x05
#define LED_RGB_G_BIT                       0x06
#define LED_RGB_B_BIT                       0x07

const uint8_t ledTransitionScale[NUMBER_OF_LED_TRANSITIONS] =
{
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    1,
    2,
    2,
    2,
    2,
    3,
    3,
    4,
    4,
    5,
    5,
    6,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    16,
    17,
    19,
    21,
    23,
    25,
    28,
    30,
    33,
    36,
    40,
    44,
    48,
    52,
    57,
    62,
    68,
    74,
    81,
    89,
    97,
    106,
    115,
    126,
    138,
    150,
    164,
    179,
    195,
    213,
    255
};
