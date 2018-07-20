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

#include "../Variables.h"

uint8_t Board::getEncoderPair(uint8_t buttonID)
{
    return buttonID/2;
}

int8_t Board::getEncoderState(uint8_t encoderID, uint8_t pulsesPerStep)
{
    //find array index of digital input buffer where data is stored for requested encoder
    uint8_t buttonID = encoderID*2;
    uint8_t arrayIndex = buttonID/8;

    uint8_t pairState = BIT_READ(digitalInBufferReadOnly[arrayIndex], 7-buttonID);
    pairState <<= 1;
    pairState |= BIT_READ(digitalInBufferReadOnly[arrayIndex], 6-buttonID);
    return readEncoder(encoderID, pairState, pulsesPerStep);
}