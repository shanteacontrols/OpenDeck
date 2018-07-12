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

#include "DigitalInput.h"
#include "board/Board.h"
#include "Variables.h"
#include "database/Database.h"

///
/// \brief Used for buttonPCinc/buttonPCdec messages when each button press/encoder rotation sends incremented or decremented PC value.
/// 16 entries in array are used for 16 MIDI channels.
///
uint8_t     lastPCvalue[16];

///
/// \brief Array holding state for all encoders (whether they're enabled or disabled).
///
uint8_t     encoderEnabled[MAX_NUMBER_OF_ENCODERS/8+1];


///
/// \brief Default constructor.
///
DigitalInput::DigitalInput()
{
    
}

///
/// \brief Used to store specific parameters from EEPROM to internal arrays for faster access.
///
void DigitalInput::init()
{
    //store some parameters from eeprom to ram for faster access
    for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
    {
        uint8_t arrayIndex = i/8;
        uint8_t encoderIndex = i - 8*arrayIndex;

        BIT_WRITE(encoderEnabled[arrayIndex], encoderIndex, database.read(DB_BLOCK_ENCODERS, dbSection_encoders_enable, i));
    }

    buttons.init();
    encoders.init();
}

///
/// \brief Checks if encoder is enabled or disabled.
/// @param [in] encoderID    Index of encoder which is being checked.
/// \returns True if encoder is enabled, false otherwise.
///
bool DigitalInput::isEncoderEnabled(uint8_t encoderID)
{
    uint8_t arrayIndex = encoderID/8;
    uint8_t encoderIndex = encoderID - 8*arrayIndex;

    return BIT_READ(encoderEnabled[arrayIndex], encoderIndex);
}

///
/// \brief Used to check if button and encoder data is available.
/// Data source (pins) for both buttons and encoders is same.
///
void DigitalInput::update()
{
    if (!board.digitalInputDataAvailable())
        return;

    buttons.update();
    encoders.update();
}

DigitalInput digitalInput;