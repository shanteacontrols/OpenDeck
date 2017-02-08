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

#include "Pins.h"

//match pins on pcb

#define MUX_Y0                      8
#define MUX_Y1                      9
#define MUX_Y2                      10
#define MUX_Y3                      11
#define MUX_Y4                      12
#define MUX_Y5                      13
#define MUX_Y6                      14
#define MUX_Y7                      15
#define MUX_Y8                      7
#define MUX_Y9                      6
#define MUX_Y10                     5
#define MUX_Y11                     4
#define MUX_Y12                     3
#define MUX_Y13                     2
#define MUX_Y14                     1
#define MUX_Y15                     0

const uint8_t muxPinOrderArray[] =
{
    MUX_Y0,
    MUX_Y1,
    MUX_Y2,
    MUX_Y3,
    MUX_Y4,
    MUX_Y5,
    MUX_Y6,
    MUX_Y7,
    MUX_Y8,
    MUX_Y9,
    MUX_Y10,
    MUX_Y11,
    MUX_Y12,
    MUX_Y13,
    MUX_Y14,
    MUX_Y15
};

#define DM_ROW_8_BIT                4
#define DM_ROW_7_BIT                5
#define DM_ROW_6_BIT                6
#define DM_ROW_5_BIT                7
#define DM_ROW_4_BIT                3
#define DM_ROW_3_BIT                2
#define DM_ROW_2_BIT                1
#define DM_ROW_1_BIT                0

const uint8_t dmRowBitArray[] =
{
    DM_ROW_1_BIT,
    DM_ROW_2_BIT,
    DM_ROW_3_BIT,
    DM_ROW_4_BIT,
    DM_ROW_5_BIT,
    DM_ROW_6_BIT,
    DM_ROW_7_BIT,
    DM_ROW_8_BIT
};

#define DM_COLUMN_1                 0
#define DM_COLUMN_2                 7
#define DM_COLUMN_3                 2
#define DM_COLUMN_4                 1
#define DM_COLUMN_5                 4
#define DM_COLUMN_6                 3
#define DM_COLUMN_7                 5
#define DM_COLUMN_8                 6

const uint8_t dmColumnArray[] =
{
    DM_COLUMN_1,
    DM_COLUMN_2,
    DM_COLUMN_3,
    DM_COLUMN_4,
    DM_COLUMN_5,
    DM_COLUMN_6,
    DM_COLUMN_7,
    DM_COLUMN_8
};
