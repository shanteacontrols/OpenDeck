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

#include "Board.h"
#include "Variables.h"

bool                dmBufferCopied;
uint64_t            inputMatrixBufferCopy;

volatile uint64_t   inputBuffer[DIGITAL_BUFFER_SIZE];
volatile uint8_t    digital_buffer_head;
volatile uint8_t    digital_buffer_tail;
volatile uint8_t    activeButtonColumn;

int8_t getInputMatrixBufferSize()
{
    uint8_t head, tail;

    head = digital_buffer_head;
    tail = digital_buffer_tail;
    if (head >= tail)
    return head - tail;

    return DIGITAL_BUFFER_SIZE + head - tail;
}

bool Board::copyInputMatrixBuffer()
{
    int8_t bufferSize = getInputMatrixBufferSize();

    if (bufferSize <= 0)
    return false;

    //some data in buffer
    //copy oldest member of buffer to inputMatrixBufferCopy
    if (digital_buffer_head == digital_buffer_tail)
    return false;

    uint8_t index = digital_buffer_tail + 1;
    if (index >= DIGITAL_BUFFER_SIZE)
    index = 0;

    cli();
    inputMatrixBufferCopy = inputBuffer[index];
    sei();

    dmBufferCopied = true;
    buttonsProcessed = false;
    encodersProcessed = false;
    digital_buffer_tail = index;

    return true;
}

void Board::checkInputMatrixBufferCopy()
{
    if ((buttonsProcessed == true) && (encodersProcessed == true) && (dmBufferCopied == true))
    {
        dmBufferCopied = false;
        buttonsProcessed = false;
        encodersProcessed = false;
    }
}
