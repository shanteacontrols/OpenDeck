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

#include "Board.h"
#include "Variables.h"

bool                encodersProcessed;
uint16_t            encoderData[MAX_NUMBER_OF_ENCODERS];

void Board::initEncoders()
{
    for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
    {
        encoderData[i] |= ((uint16_t)0 << 8);
        encoderData[i] |= ((uint16_t)ENCODER_DEFAULT_PULSE_COUNT_STATE << 4);   //set number of pulses to 8
    }
}

uint8_t Board::getEncoderPair(uint8_t buttonIndex)
{
    return buttonIndex/2;
}

bool Board::encoderDataAvailable()
{
    return false;
}

int8_t Board::getEncoderState(uint8_t encoderNumber)
{
    return 0;
}

inline int8_t readEncoder(uint8_t encoderID, uint8_t pairState)
{
    return 0;
}

#endif