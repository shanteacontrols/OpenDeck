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

#include "board/Board.h"

///
/// \brief Size of array used to store all digital input readings.
/// Values are stored in byte array - one bit represents single digital input value.
///
#ifndef IN_MATRIX
#if ((MAX_NUMBER_OF_BUTTONS % 8) != 0)
#define DIGITAL_IN_ARRAY_SIZE   ((MAX_NUMBER_OF_BUTTONS / 8) + 1)
#else
#define DIGITAL_IN_ARRAY_SIZE   (MAX_NUMBER_OF_BUTTONS / 8)
#endif
#else
#define DIGITAL_IN_ARRAY_SIZE   NUMBER_OF_BUTTON_COLUMNS
#endif

///
/// \brief Size of ring buffer used to store all digital input readings.
/// Once digital input array is full (all inputs are read), index within ring buffer
/// is incremented (if there is space left).
///
#define DIGITAL_IN_BUFFER_SIZE  5
