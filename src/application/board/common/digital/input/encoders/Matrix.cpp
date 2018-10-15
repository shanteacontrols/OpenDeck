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

#include "Common.h"
#include "board/common/digital/input/Variables.h"

namespace Board
{
    uint8_t getEncoderPair(uint8_t buttonID)
    {
        uint8_t row = buttonID/NUMBER_OF_BUTTON_COLUMNS;
        uint8_t column = buttonID % NUMBER_OF_BUTTON_COLUMNS;

        if (row%2)
            row -= 1;   //uneven row, get info from previous (even) row

        return (row*NUMBER_OF_BUTTON_COLUMNS)/2 + column;
    }

    encoderPosition_t getEncoderState(uint8_t encoderID, uint8_t pulsesPerStep)
    {
        using namespace Board::detail;

        uint8_t column = encoderID % NUMBER_OF_BUTTON_COLUMNS;
        uint8_t row  = (encoderID/NUMBER_OF_BUTTON_COLUMNS)*2;
        uint8_t pairState = (digitalInBufferReadOnly[column] >> row) & 0x03;

        return readEncoder(encoderID, pairState, pulsesPerStep);
    }
}