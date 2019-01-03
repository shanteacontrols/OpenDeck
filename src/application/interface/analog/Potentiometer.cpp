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
    uint16_t maxLimit;
    uint16_t stepDiff;

    if ((analogType == aType_NRPN_14) || (analogType == aType_PitchBend))
    {
        maxLimit = MIDI_14_BIT_VALUE_MAX;
        stepDiff = ANALOG_STEP_MIN_DIFF_14_BIT;
    }
    else
    {
        maxLimit = MIDI_7_BIT_VALUE_MAX;
        stepDiff = ANALOG_STEP_MIN_DIFF_7_BIT;
    }

    bool direction = value > lastAnalogueValue[analogID];

    //when potentiometer changes direction, use double step difference to avoid jumping of values
    if (direction != lastDirection[analogID])
        stepDiff *= 2;

    if (abs(value - lastAnalogueValue[analogID]) < stepDiff)
        return;

    lastDirection[analogID] = direction;

    uint16_t midiValue = mapRange_uint32(value, 0, ADC_MAX_VALUE, 0, maxLimit);
    uint16_t oldMIDIvalue = mapRange_uint32(lastAnalogueValue[analogID], 0, ADC_MAX_VALUE, 0, maxLimit);

    if (midiValue == oldMIDIvalue)
        return;

    uint16_t lowerLimit = database.read(DB_BLOCK_ANALOG, dbSection_analog_lowerLimit, analogID);
    uint16_t upperLimit = database.read(DB_BLOCK_ANALOG, dbSection_analog_upperLimit, analogID);
    uint16_t midiID = database.read(DB_BLOCK_ANALOG, dbSection_analog_midiID, analogID);
    uint8_t channel = database.read(DB_BLOCK_ANALOG, dbSection_analog_midiChannel, analogID);
    uint16_t scaledMIDIvalue;
    encDec_14bit_t encDec_14bit;

    if
    (
        (analogType == aType_potentiometer_cc) || 
        (analogType == aType_potentiometer_note) || 
        (analogType == aType_NRPN_7)
    )
    {
        //use 7-bit MIDI ID and limits
        encDec_14bit.value = midiID;
        encDec_14bit.split14bit();
        midiID = encDec_14bit.low;

        encDec_14bit.value = lowerLimit;
        encDec_14bit.split14bit();
        lowerLimit = encDec_14bit.low;

        encDec_14bit.value = upperLimit;
        encDec_14bit.split14bit();
        upperLimit = encDec_14bit.low;
    }
    // else
    // {
    //     //14-bit values are already read
    // }

    //use mapRange_uint32 to avoid overflow issues
    scaledMIDIvalue = mapRange_uint32(midiValue, 0, maxLimit, lowerLimit, upperLimit);

    //invert MIDI data if configured
    if (database.read(DB_BLOCK_ANALOG, dbSection_analog_invert, analogID))
    {
        if ((analogType == aType_NRPN_14) || (analogType == aType_PitchBend))
            scaledMIDIvalue = maxLimit - scaledMIDIvalue;
    }

    switch(analogType)
    {
        case aType_potentiometer_cc:
        case aType_potentiometer_note:
        if (analogType == aType_potentiometer_cc)
        {
            midi.sendControlChange(midiID, scaledMIDIvalue, channel);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageControlChange_display, midiID, scaledMIDIvalue, channel+1);
            #endif
        }
        else
        {
            midi.sendNoteOn(midiID, scaledMIDIvalue, channel);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageNoteOn_display, midiID, scaledMIDIvalue, channel+1);
            #endif
        }
        break;

        case aType_NRPN_7:
        case aType_NRPN_14:
        //when nrpn is used, MIDI ID is split into two messages
        //first message contains higher byte
        encDec_14bit.value = midiID;
        encDec_14bit.split14bit();
        midi.sendControlChange(99, encDec_14bit.high, channel);
        midi.sendControlChange(98, encDec_14bit.low, channel);

        if (analogType == aType_NRPN_7)
        {
            midi.sendControlChange(6, scaledMIDIvalue, channel);
        }
        else
        {
            //send 14-bit NRPN value in another two messages
            //first message contains higher byte
            encDec_14bit.value = scaledMIDIvalue;
            encDec_14bit.split14bit();

            midi.sendControlChange(6, encDec_14bit.high, channel);
            midi.sendControlChange(38, encDec_14bit.low, channel);
        }

        #ifdef DISPLAY_SUPPORTED
        display.displayMIDIevent(displayEventOut, midiMessageNRPN_display, midiID, scaledMIDIvalue, channel+1);
        #endif
        break;

        case aType_PitchBend:
        midi.sendPitchBend(scaledMIDIvalue, channel);
        #ifdef DISPLAY_SUPPORTED
        display.displayMIDIevent(displayEventOut, midiMessagePitchBend_display, midiID, scaledMIDIvalue, channel+1);
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
