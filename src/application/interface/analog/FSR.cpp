/*

Copyright 2015-2019 Igor Petrovic

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
#include "board/Board.h"
#include "core/src/general/BitManipulation.h"
#include "core/src/general/Misc.h"

using namespace Interface::analog;

//use 1k resistor when connecting FSR between signal and ground

uint32_t Analog::calibratePressure(uint32_t value, pressureType_t type)
{
    switch (type)
    {
    case pressureType_t::velocity:
        return core::misc::mapRange(CONSTRAIN(value, static_cast<uint32_t>(FSR_MIN_VALUE), static_cast<uint32_t>(FSR_MAX_VALUE)), static_cast<uint32_t>(FSR_MIN_VALUE), static_cast<uint32_t>(FSR_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX));

    case pressureType_t::aftertouch:
        return core::misc::mapRange(CONSTRAIN(value, static_cast<uint32_t>(FSR_MIN_VALUE), static_cast<uint32_t>(AFTERTOUCH_MAX_VALUE)), static_cast<uint32_t>(FSR_MIN_VALUE), static_cast<uint32_t>(AFTERTOUCH_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX));

    default:
        return 0;
    }
}

bool Analog::getFsrPressed(uint8_t fsrID)
{
    return fsrPressed[fsrID];
}

void Analog::setFsrPressed(uint8_t fsrID, bool state)
{
    fsrPressed[fsrID] = state;
}

void Analog::checkFSRvalue(uint8_t analogID, uint16_t pressure)
{
    auto calibratedPressure = calibratePressure(pressure, pressureType_t::velocity);

    if (calibratedPressure > 0)
    {
        if (!getFsrPressed(analogID))
        {
            //sensor is really pressed
            setFsrPressed(analogID, true);
            uint8_t note = database.read(DB_BLOCK_ANALOG, dbSection_analog_midiID, analogID);
            uint8_t channel = database.read(DB_BLOCK_ANALOG, dbSection_analog_midiChannel, analogID);
            midi.sendNoteOn(note, calibratedPressure, channel);
#ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::noteOn, note, calibratedPressure, channel + 1);
#endif
#ifdef LEDS_SUPPORTED
            leds.midiToState(MIDI::messageType_t::noteOn, note, calibratedPressure, channel, true);
#endif

            cInfo.send(DB_BLOCK_ANALOG, analogID);
        }
    }
    else
    {
        if (getFsrPressed(analogID))
        {
            setFsrPressed(analogID, false);
            uint8_t note = database.read(DB_BLOCK_ANALOG, dbSection_analog_midiID, analogID);
            uint8_t channel = database.read(DB_BLOCK_ANALOG, dbSection_analog_midiChannel, analogID);
            midi.sendNoteOff(note, 0, channel);
#ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::noteOff, note, calibratedPressure, channel + 1);
#endif
#ifdef LEDS_SUPPORTED
            leds.midiToState(MIDI::messageType_t::noteOff, 0, channel, true);
#endif

            cInfo.send(DB_BLOCK_ANALOG, analogID);
        }
    }

    //update values
    lastAnalogueValue[analogID] = pressure;
}