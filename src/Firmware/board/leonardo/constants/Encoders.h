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

#ifdef BOARD_A_LEO

#pragma once

/*
    Encoder data formatting, uint16_t variable type
    0      1      2      3
    0000 | 0000 | 0000 | 0000

    0 - encoder direction (0/1 - left/right)
    1 - encoderMoving (0/1/2 - stopped/left/right)
    2 - counted pulses (default value is 8 to avoid issues with negative values)
    3 - temp encoder state (2 readings of 2 encoder pairs)
*/

#define ENCODER_CLEAR_TEMP_STATE_MASK       0xFFF0
#define ENCODER_CLEAR_PULSES_MASK           0xFF0F
#define ENCODER_CLEAR_MOVING_STATUS_MASK    0xF0FF
#define ENCODER_DIRECTION_BIT               15
#define ENCODER_DEFAULT_PULSE_COUNT_STATE   8
#define PULSES_PER_STEP                     4

const int8_t encoderLookUpTable[] =
{
    0,
    1,
    -1,
    2,
    -1,
    0,
    -2,
    1,
    1,
    -2,
    0,
    -1,
    2,
    -1,
    1,
    0
};

#endif