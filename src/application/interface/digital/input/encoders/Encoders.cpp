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

#include "Encoders.h"
#include "interface/digital/input/Common.h"

///
/// \brief Continuously checks state of all encoders.
///
void Encoders::update()
{
    uint8_t encoderValue;

    for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
    {
        if (!database.read(DB_BLOCK_ENCODERS, dbSection_encoders_enable, i))
            continue;

        encoderPosition_t encoderState = board.getEncoderState(i, database.read(DB_BLOCK_ENCODERS, dbSection_encoders_pulsesPerStep, i));

        if (encoderState == encStopped)
            continue;

        if (database.read(DB_BLOCK_ENCODERS, dbSection_encoders_invert, i))
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

            case encTypeCC:
            static uint8_t ccValue[MAX_NUMBER_OF_ENCODERS] = { 0 };

            if (encoderState == encMoveLeft)
            {
                if (ccValue[i])
                    ccValue[i]--;
            }
            else
            {
                if (ccValue[i] < 127)
                    ccValue[i]++;
            }

            encoderValue = ccValue[i];
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

        if (cinfoHandler != nullptr)
            (*cinfoHandler)(DB_BLOCK_ENCODERS, i);
    }
}

void Encoders::setCinfoHandler(bool(*fptr)(dbBlockID_t dbBlock, sysExParameter_t componentID))
{
    cinfoHandler = fptr;
}