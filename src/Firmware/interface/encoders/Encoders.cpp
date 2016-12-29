/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015, 2016 Igor Petrovic

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
#include "../../database/Database.h"
#include "../../midi/MIDI.h"
#include "Constants.h"

Encoders::Encoders()
{
    //def const
}

void Encoders::update()
{
    if (!board.encoderDataAvailable())
        return;

    uint8_t encoderValue;

    for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
    {
        if (!database.read(CONF_BLOCK_ENCODER, encoderEnabledSection, i))
            continue;

        encoderPosition_t encoderState = (encoderPosition_t)board.getEncoderState(i);
        if (encoderState == encStopped)
            continue;

        if (database.read(CONF_BLOCK_ENCODER, encoderInvertedSection, i))
        {
            if (encoderState == encMoveLeft)
                encoderState = encMoveRight;
             else
                encoderState = encMoveLeft;
        }

        switch((encoderType_t)database.read(CONF_BLOCK_ENCODER, encoderEncodingModeSection, i))
        {
            case enc7Fh01h:
            if (encoderState == encMoveLeft)
                encoderValue = ENCODER_VALUE_LEFT_7FH01H;
            else
                encoderValue = ENCODER_VALUE_RIGHT_7FH01H;
            break;

            case enc3Fh41h:
            if (encoderState == encMoveLeft)
                encoderValue = ENCODER_VALUE_LEFT_3FH41H;
            else
                encoderValue = ENCODER_VALUE_RIGHT_3FH41H;
            break;

            default:
            continue;
            break;
        }

        midi.sendControlChange(database.read(CONF_BLOCK_ENCODER, encoderMIDIidSection, i), encoderValue, database.read(CONF_BLOCK_MIDI, midiChannelSection, CCchannel));
        //if (sysEx.configurationEnabled())
            //sysEx.sendComponentID(CONF_BLOCK_ENCODER, i);
    }
}

Encoders encoders;
