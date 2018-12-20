/*

Copyright 2015-2018 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "Analog.h"
#include "core/src/general/Misc.h"
#include "interface/CInfo.h"

void Analog::checkPotentiometerValue(analogType_t analogType, uint8_t analogID, uint16_t value)
{
    if (abs(value - lastAnalogueValue[analogID]) < ANALOG_STEP_MIN_DIFF)
        return;

    uint16_t midiValue;
    uint16_t oldMIDIvalue;

    midiValue = Board::scaleADC(value, ((analogType == aType_NRPN_14) || (analogType == aType_PitchBend)) ? MIDI_14_BIT_VALUE_MAX : MIDI_7_BIT_VALUE_MAX);
    oldMIDIvalue = Board::scaleADC(lastAnalogueValue[analogID], ((analogType == aType_NRPN_14) || (analogType == aType_PitchBend)) ? MIDI_14_BIT_VALUE_MAX : MIDI_7_BIT_VALUE_MAX);

    if (midiValue == oldMIDIvalue)
        return;

    uint16_t lowerCClimit_14bit = database.read(DB_BLOCK_ANALOG, dbSection_analog_lowerLimit, analogID);
    uint16_t upperCClimit_14bit = database.read(DB_BLOCK_ANALOG, dbSection_analog_upperLimit, analogID);

    encDec_14bit_t encDec_14bit;

    encDec_14bit.value = lowerCClimit_14bit;
    encDec_14bit.split14bit();

    uint8_t lowerCClimit_7bit = encDec_14bit.low;

    encDec_14bit.value = upperCClimit_14bit;
    encDec_14bit.split14bit();

    uint8_t upperCClimit_7bit = encDec_14bit.low;

    //invert CC data if configured
    if (database.read(DB_BLOCK_ANALOG, dbSection_analog_invert, analogID))
    {
        if ((analogType == aType_NRPN_14) || (analogType == aType_PitchBend))
            midiValue = 16383 - midiValue;
        else
            midiValue = 127 - midiValue;
    }

    uint8_t midiID = database.read(DB_BLOCK_ANALOG, dbSection_analog_midiID, analogID);
    uint8_t channel = database.read(DB_BLOCK_ANALOG, dbSection_analog_midiChannel, analogID);
    uint8_t sendVal = mapRange_uint8(midiValue, 0, 127, lowerCClimit_7bit, upperCClimit_7bit);

    encDec_14bit.value = midiID;
    encDec_14bit.split14bit();

    switch(analogType)
    {
        case aType_potentiometer_cc:
        case aType_potentiometer_note:
        midiID = encDec_14bit.low;

        if (analogType == aType_potentiometer_cc)
        {
            midi.sendControlChange(midiID, sendVal, channel);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageControlChange_display, midiID, sendVal, channel+1);
            #endif
        }
        else
        {
            midi.sendNoteOn(midiID, sendVal, channel);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageControlChange_display, midiID, sendVal, channel+1);
            #endif
        }
        break;

        case aType_NRPN_7:
        case aType_NRPN_14:
        midi.sendControlChange(99, encDec_14bit.high, channel);
        midi.sendControlChange(98, encDec_14bit.low, channel);

        if (analogType == aType_NRPN_7)
        {
            uint8_t value = mapRange_uint8(midiValue, 0, MIDI_7_BIT_VALUE_MAX, lowerCClimit_7bit, upperCClimit_7bit);
            midi.sendControlChange(6, value, channel);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageNRPN_display, encDec_14bit.value, value, channel+1);
            #endif
        }
        else
        {
            //use mapRange_uint32 to avoid overflow issues
            encDec_14bit.value = mapRange_uint32(midiValue, 0, MIDI_14_BIT_VALUE_MAX, lowerCClimit_14bit, upperCClimit_14bit);
            encDec_14bit.split14bit();

            midi.sendControlChange(6, encDec_14bit.high, channel);
            midi.sendControlChange(38, encDec_14bit.low, channel);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageNRPN_display, database.read(DB_BLOCK_ANALOG, dbSection_analog_midiID, analogID), encDec_14bit.value, channel+1);
            #endif
        }
        break;

        case aType_PitchBend:
        //use mapRange_uint32 to avoid overflow issues
        encDec_14bit.value = mapRange_uint32(midiValue, 0, MIDI_14_BIT_VALUE_MAX, lowerCClimit_14bit, upperCClimit_14bit);
        midi.sendPitchBend(midiValue, channel);
        #ifdef DISPLAY_SUPPORTED
        display.displayMIDIevent(displayEventOut, midiMessagePitchBend_display, database.read(DB_BLOCK_ANALOG, dbSection_analog_midiID, analogID), encDec_14bit.value, channel+1);
        #endif
        break;

        default:
        return;
    }

    if (cinfoHandler != nullptr)
        (*cinfoHandler)(DB_BLOCK_ANALOG, analogID);

    //update values
    lastAnalogueValue[analogID] = value;
}
