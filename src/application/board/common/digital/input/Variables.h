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

#include <inttypes.h>
#include "board/common/constants/DigitalIn.h"

namespace Board
{
    namespace detail
    {
        ///
        /// \brief Array used to store readings from digital input matrix.
        ///
        extern volatile uint8_t     digitalInBuffer[DIGITAL_IN_BUFFER_SIZE][DIGITAL_IN_ARRAY_SIZE];

        ///
        /// \brief Read only copy of digital input buffer.
        /// Used to avoid data overwrite from ISR.
        ///
        extern uint8_t              digitalInBufferReadOnly[DIGITAL_IN_ARRAY_SIZE];

        #ifdef IN_MATRIX
        ///
        /// \brief Holds value of currently active input matrix column.
        ///
        extern volatile uint8_t     activeInColumn;
        #endif

        ///
        /// \brief Holds "head" index position in ring buffer.
        ///
        extern volatile uint8_t     dIn_head;

        ///
        /// \brief Holds "tail" index position in ring buffer.
        ///
        extern volatile uint8_t     dIn_tail;

        ///
        /// \brief Holds current number of elements stored in ring buffer.
        ///
        extern volatile uint8_t     dIn_count;
    }
}