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

#include "Constants.h"
#include "board/Board.h"
#include "../../../../database/Database.h"
#include "sysex/src/SysEx.h"
#include "../../../cinfo/CInfo.h"
#include "../Variables.h"
#ifdef DISPLAY_SUPPORTED
#include "../../../display/Display.h"
#endif
#include "../DigitalInput.h"


///
/// \brief Array holding invert state for all encoders.
///
uint8_t     encoderInverted[MAX_NUMBER_OF_ENCODERS/8+1];

///
/// \brief Array holding state for all encoders (whether they're enabled or disabled).
///
uint8_t     encoderEnabled[MAX_NUMBER_OF_ENCODERS/8+1];

///
/// \brief Holds pulses per step for all encoders.
///
uint8_t     pulsesPerStep[MAX_NUMBER_OF_ENCODERS];

///
/// \brief Array used for easier access to current encoder MIDI value in 7Fh01h and 3Fh41h modes.
/// Matched with encoderType_t and encoderPosition_t
///
const uint8_t encValue[2][3] =
{
    {
        0,
        ENCODER_VALUE_LEFT_7FH01H,
        ENCODER_VALUE_RIGHT_7FH01H
    },

    {
        0,
        ENCODER_VALUE_LEFT_3FH41H,
        ENCODER_VALUE_RIGHT_3FH41H
    }
};

///
/// \brief Checks if encoder is inverted.
/// @param [in] encoderID    Index of encoder which is being checked.
/// \returns True if encoder is inverted, false otherwise.
///
inline bool isEncoderInverted(uint8_t encoderID)
{
    uint8_t arrayIndex = encoderID/8;
    uint8_t encoderIndex = encoderID - 8*arrayIndex;

    return BIT_READ(encoderInverted[arrayIndex], encoderIndex);
}

///
/// \brief Default constructor.
///
Encoders::Encoders()
{

}

///
/// \brief Used to store specific parameters from EEPROM to internal arrays for faster access.
///
void Encoders::init()
{
    //store some parameters from eeprom to ram for faster access
    for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
    {
        uint8_t arrayIndex = i/8;
        uint8_t encoderIndex = i - 8*arrayIndex;

        BIT_WRITE(encoderEnabled[arrayIndex], encoderIndex, database.read(DB_BLOCK_ENCODERS, dbSection_encoders_enable, i));
        BIT_WRITE(encoderInverted[arrayIndex], encoderIndex, database.read(DB_BLOCK_ENCODERS, dbSection_encoders_invert, i));
        pulsesPerStep[i] = database.read(DB_BLOCK_ENCODERS, dbSection_encoders_pulsesPerStep, i);
    }
}

///
/// \brief Continuously checks state of all encoders.
///
void Encoders::update()
{
    uint8_t encoderValue;

    for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
    {
        if (!isEncoderEnabled(i))
            continue;

        encoderPosition_t encoderState = (encoderPosition_t)board.getEncoderState(i, pulsesPerStep[i]);

        if (encoderState == encStopped)
            continue;

        if (isEncoderInverted(i))
        {
            if (encoderState == encMoveLeft)
                encoderState = encMoveRight;
             else
                encoderState = encMoveLeft;
        }

        uint8_t midiID = database.read(DB_BLOCK_ENCODERS, dbSection_encoders_midiID, i);
        uint8_t channel = database.read(DB_BLOCK_ENCODERS, dbSection_encoders_midiChannel, i);
        encoderType_t type = (encoderType_t)database.read(DB_BLOCK_ENCODERS, dbSection_encoders_mode, i);

        switch(type)
        {
            case encType7Fh01h:
            case encType3Fh41h:
            encoderValue = encValue[(uint8_t)type][(uint8_t)encoderState];
            break;

            case encTypePC:
            if (encoderState == encMoveLeft)
            {
                if (lastPCvalue[channel] < 127)
                    lastPCvalue[channel]++;
            }
            else
            {
                if (lastPCvalue[channel] > 0)
                    lastPCvalue[channel]--;
            }

            encoderValue = lastPCvalue[channel];
            break;

            default:
            continue;
            break;
        }

        if (type == encTypePC)
        {
            midi.sendProgramChange(encoderValue, channel);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageProgramChange_display, midiID & 0x7F, encoderValue, channel+1);
            #endif
        }
        else
        {
            midi.sendControlChange(midiID, encoderValue, channel);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageControlChange_display, midiID & 0x7F, encoderValue, channel+1);
            #endif
        }

        sendCinfo(DB_BLOCK_ENCODERS, i);
    }
}

///
/// \brief Checks if encoder is enabled or disabled.
/// @param [in] encoderID    Index of encoder which is being checked.
/// \returns True if encoder is enabled, false otherwise.
///
bool Encoders::isEncoderEnabled(uint8_t encoderID)
{
    uint8_t arrayIndex = encoderID/8;
    uint8_t encoderIndex = encoderID - 8*arrayIndex;

    return BIT_READ(encoderEnabled[arrayIndex], encoderIndex);
}

Encoders encoders;
