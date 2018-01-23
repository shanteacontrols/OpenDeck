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

#include "Analog.h"
#include "../../board/Board.h"
#include "sysex/src/SysEx.h"
#include "../cinfo/CInfo.h"

void Analog::checkPotentiometerValue(uint8_t analogID, uint16_t value)
{
    analogType_t analogType = (analogType_t)database.read(DB_BLOCK_ANALOG, analogTypeSection, analogID);
    uint16_t midiValue;
    uint16_t oldMIDIvalue;


    uint16_t lowerCClimit_14bit = database.read(DB_BLOCK_ANALOG, analogCClowerLimitSection, analogID);
    uint16_t upperCClimit_14bit = database.read(DB_BLOCK_ANALOG, analogCCupperLimitSection, analogID);

    uint8_t lowerCClimit_7bit = lowerCClimit_14bit & (uint16_t)0x00FF;
    uint8_t upperCClimit_7bit = upperCClimit_14bit & (uint16_t)0x00FF;

    encDec_14bit_t encDec_14bit;

    midiValue = board.scaleADC(value, (analogType == aType_NRPN_14) ? 16383 : 127);
    oldMIDIvalue = board.scaleADC(lastAnalogueValue[analogID], 127);

    if (midiValue == oldMIDIvalue)
        return;

    //invert CC data if potInverted is true
    if (database.read(DB_BLOCK_ANALOG, analogInvertedSection, analogID))
    {
        if (analogType == aType_NRPN_14)
            midiValue = 16383 - midiValue;
        else
            midiValue = 127 - midiValue;
    }

    uint16_t analogDiff = abs(value - lastAnalogueValue[analogID]);
    uint16_t minDiff = (analogType == aType_NRPN_14) ? ANALOG_14_BIT_STEP_MIN : ANALOG_7_BIT_STEP_MIN;

    if (analogDiff < minDiff)
        return;

    switch(analogType)
    {
        case aType_potentiometer_cc:
        case aType_potentiometer_note:

        if (analogType == aType_potentiometer_cc)
            midi.sendControlChange(database.read(DB_BLOCK_ANALOG, analogMIDIidSection, analogID), mapAnalog_uint8(midiValue, 0, 127, lowerCClimit_7bit, upperCClimit_7bit), database.read(DB_BLOCK_MIDI, midiChannelSection, CCchannel));
        else
            midi.sendNoteOn(database.read(DB_BLOCK_ANALOG, analogMIDIidSection, analogID), mapAnalog_uint8(midiValue, 0, 127, lowerCClimit_7bit, upperCClimit_7bit), database.read(DB_BLOCK_MIDI, midiChannelSection, CCchannel));
        break;

        case aType_NRPN_7:
        case aType_NRPN_14:
        encDec_14bit.value = database.read(DB_BLOCK_ANALOG, analogMIDIidSection, analogID);
        encDec_14bit.split14bit();
        midi.sendControlChange(99, encDec_14bit.high, database.read(DB_BLOCK_MIDI, midiChannelSection, CCchannel));
        midi.sendControlChange(98, encDec_14bit.low, database.read(DB_BLOCK_MIDI, midiChannelSection, CCchannel));

        if (analogType == aType_NRPN_7)
        {
            midi.sendControlChange(6, mapAnalog_uint8(midiValue, 0, 127, lowerCClimit_7bit, upperCClimit_7bit), database.read(DB_BLOCK_MIDI, midiChannelSection, CCchannel));
        }
        else
        {
            encDec_14bit.value = mapAnalog_uint16(midiValue, 0, 16383, lowerCClimit_14bit, upperCClimit_14bit);
            encDec_14bit.split14bit();

            midi.sendControlChange(6, encDec_14bit.high, database.read(DB_BLOCK_MIDI, midiChannelSection, CCchannel));
            midi.sendControlChange(38, encDec_14bit.low, database.read(DB_BLOCK_MIDI, midiChannelSection, CCchannel));
        }
        break;

        default:
        return;
    }

    if (sysEx.configurationEnabled())
    {
        if ((rTimeMs() - getLastCinfoMsgTime(DB_BLOCK_ANALOG)) > COMPONENT_INFO_TIMEOUT)
        {
            sysEx.startResponse();
            sysEx.addToResponse(COMPONENT_ID_STRING);
            sysEx.addToResponse(DB_BLOCK_ANALOG);
            sysEx.addToResponse(analogID);
            sysEx.sendResponse();
            updateCinfoTime(DB_BLOCK_ANALOG);
        }
    }

    //update values
    lastAnalogueValue[analogID] = value;
}
