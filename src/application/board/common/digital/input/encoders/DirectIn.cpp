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
#include "../Variables.h"
#include "core/src/general/BitManipulation.h"
#include "board/Board.h"

namespace Board
{
    uint8_t getEncoderPair(uint8_t buttonID)
    {
        return buttonID/2;
    }

    encoderPosition_t getEncoderState(uint8_t encoderID, uint8_t pulsesPerStep)
    {
        using namespace Board::detail;

        uint8_t buttonID = encoderID*2;

        uint8_t pairState = Board::getButtonState(buttonID);
        pairState <<= 1;
        pairState |= Board::getButtonState(buttonID+1);

        return readEncoder(encoderID, pairState, pulsesPerStep);
    }
}