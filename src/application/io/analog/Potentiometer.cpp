/*

Copyright 2015-2020 Igor Petrovic

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
#include "core/src/general/Helpers.h"

using namespace IO;

void Analog::checkPotentiometerValue(type_t analogType, uint8_t analogID, uint32_t value)
{
    uint16_t maxLimit;

    uint16_t             lowerLimit = database.read(Database::Section::analog_t::lowerLimit, analogID);
    uint16_t             upperLimit = database.read(Database::Section::analog_t::upperLimit, analogID);
    uint16_t             midiID     = database.read(Database::Section::analog_t::midiID, analogID);
    uint8_t              channel    = database.read(Database::Section::analog_t::midiChannel, analogID);
    MIDI::encDec_14bit_t encDec_14bit;

    if ((analogType == type_t::nrpn14b) || (analogType == type_t::pitchBend) || (analogType == type_t::cc14bit))
    {
        //14-bit values are already read
        maxLimit = MIDI_14_BIT_VALUE_MAX;
    }
    else
    {
        maxLimit = MIDI_7_BIT_VALUE_MAX;

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

    if (value > maxLimit)
        return;

    uint32_t scaledMIDIvalue;

    if (lowerLimit > upperLimit)
    {
        scaledMIDIvalue = core::misc::mapRange(value, static_cast<uint32_t>(0), static_cast<uint32_t>(maxLimit), static_cast<uint32_t>(upperLimit), static_cast<uint32_t>(lowerLimit));

        if (!database.read(Database::Section::analog_t::invert, analogID))
            scaledMIDIvalue = upperLimit - (scaledMIDIvalue - lowerLimit);
    }
    else
    {
        scaledMIDIvalue = core::misc::mapRange(value, static_cast<uint32_t>(0), static_cast<uint32_t>(maxLimit), static_cast<uint32_t>(lowerLimit), static_cast<uint32_t>(upperLimit));

        if (database.read(Database::Section::analog_t::invert, analogID))
            scaledMIDIvalue = upperLimit - (scaledMIDIvalue - lowerLimit);
    }

    if (scaledMIDIvalue == lastValue[analogID])
        return;

    switch (analogType)
    {
    case type_t::potentiometerControlChange:
    case type_t::potentiometerNote:
        if (analogType == type_t::potentiometerControlChange)
        {
            midi.sendControlChange(midiID, scaledMIDIvalue, channel);
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::controlChange, midiID, scaledMIDIvalue, channel + 1);
        }
        else
        {
            midi.sendNoteOn(midiID, scaledMIDIvalue, channel);
#ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::noteOn, midiID, scaledMIDIvalue, channel + 1);
#endif
        }
        break;

    case type_t::nrpn7b:
    case type_t::nrpn14b:
    case type_t::cc14bit:
        //when nrpn/cc14bit is used, MIDI ID is split into two messages
        //first message contains higher byte

        encDec_14bit.value = midiID;
        encDec_14bit.split14bit();

        if (analogType != type_t::cc14bit)
        {
            midi.sendControlChange(99, encDec_14bit.high, channel);
            midi.sendControlChange(98, encDec_14bit.low, channel);
        }

        if (analogType == type_t::nrpn7b)
        {
            midi.sendControlChange(6, scaledMIDIvalue, channel);
        }
        else
        {
            midiID = encDec_14bit.low;

            //send 14-bit value in another two messages
            //first message contains higher byte
            encDec_14bit.value = scaledMIDIvalue;
            encDec_14bit.split14bit();

            if (analogType == type_t::cc14bit)
            {
                if (midiID >= 96)
                    break;    //not allowed

                midi.sendControlChange(midiID, encDec_14bit.high, channel);
                midi.sendControlChange(midiID + 32, encDec_14bit.low, channel);
            }
            else
            {
                midi.sendControlChange(6, encDec_14bit.high, channel);
                midi.sendControlChange(38, encDec_14bit.low, channel);
            }
        }

        display.displayMIDIevent(Display::eventType_t::out, (analogType == type_t::cc14bit) ? Display::event_t::controlChange : Display::event_t::nrpn, midiID, scaledMIDIvalue, channel + 1);
        break;

    case type_t::pitchBend:
        midi.sendPitchBend(scaledMIDIvalue, channel);
        display.displayMIDIevent(Display::eventType_t::out, Display::event_t::pitchBend, midiID, scaledMIDIvalue, channel + 1);
        break;

    default:
        return;
    }

    lastValue[analogID] = scaledMIDIvalue;
}
