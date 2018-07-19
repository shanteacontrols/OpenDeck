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

#include "Variables.h"


volatile uint8_t    digitalInBuffer[DIGITAL_IN_BUFFER_SIZE][DIGITAL_IN_ARRAY_SIZE];

uint8_t             digitalInBufferReadOnly[DIGITAL_IN_ARRAY_SIZE];

#ifdef IN_MATRIX
volatile uint8_t    activeInColumn;
#endif


volatile uint8_t    dIn_head;
volatile uint8_t    dIn_tail;
volatile uint8_t    dIn_count;


bool Board::digitalInputDataAvailable()
{
    if (dIn_count >= DIGITAL_IN_BUFFER_SIZE)
    {
        //ring buffer is full - this section is interrupt-safe currently
        if (++dIn_tail == DIGITAL_IN_BUFFER_SIZE)
            dIn_tail = 0;

        for (int i=0; i<DIGITAL_IN_ARRAY_SIZE; i++)
            digitalInBufferReadOnly[i] = digitalInBuffer[dIn_tail][i];

        dIn_count--;

        return true;
    }

    return false;
}
