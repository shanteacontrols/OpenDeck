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

#include "board/Board.h"
#include "board/common/constants/Encoders.h"

///
/// \brief Array holding last two readings from encoder pins.
///
uint8_t encoderData[MAX_NUMBER_OF_ENCODERS];

///
/// \brief Array holding current amount of pulses for all encoders.
///
int8_t  encoderPulses[MAX_NUMBER_OF_ENCODERS];

int8_t Board::readEncoder(uint8_t encoderID, uint8_t pairState, uint8_t pulsesPerStep)
{
    int8_t returnValue = 0;

    //add new data
    encoderData[encoderID] <<= 2;
    encoderData[encoderID] |= pairState;
    encoderData[encoderID] &= 0x0F;

    encoderPulses[encoderID] += encoderLookUpTable[encoderData[encoderID]];

    if (abs(encoderPulses[encoderID]) >= pulsesPerStep)
    {
        returnValue = (encoderPulses[encoderID] > 0) ? 1 : -1;
        //reset count
        encoderPulses[encoderID] = 0;
    }

    return returnValue;
}