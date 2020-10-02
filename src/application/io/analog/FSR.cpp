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

bool Analog::getFsrPressed(uint8_t fsrID)
{
    return fsrPressed[fsrID];
}

void Analog::setFsrPressed(uint8_t fsrID, bool state)
{
    fsrPressed[fsrID] = state;
}

void Analog::checkFSRvalue(uint8_t analogID, uint32_t value)
{
    if (value > 0)
    {
        if (!getFsrPressed(analogID))
        {
            //sensor is really pressed
            setFsrPressed(analogID, true);
            uint8_t note    = database.read(Database::Section::analog_t::midiID, analogID);
            uint8_t channel = database.read(Database::Section::analog_t::midiChannel, analogID);
            midi.sendNoteOn(note, value, channel);
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::noteOn, note, value, channel + 1);
            leds.midiToState(MIDI::messageType_t::noteOn, note, value, channel, true);
        }
    }
    else
    {
        if (getFsrPressed(analogID))
        {
            setFsrPressed(analogID, false);
            uint8_t note    = database.read(Database::Section::analog_t::midiID, analogID);
            uint8_t channel = database.read(Database::Section::analog_t::midiChannel, analogID);
            midi.sendNoteOff(note, 0, channel);
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::noteOff, note, value, channel + 1);
            leds.midiToState(MIDI::messageType_t::noteOff, note, 0, channel, true);
        }
    }

    lastValue[analogID] = value;
}