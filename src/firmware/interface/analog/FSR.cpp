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
#include "../digital/output/leds/LEDs.h"
#include "sysex/src/SysEx.h"
#include "../cinfo/CInfo.h"
#include "Variables.h"

//use 1k resistor when connecting FSR between signal and ground

inline int16_t calibratePressure(int16_t value, pressureType_t type)
{
    switch(type)
    {
        case velocity:
        return mapRange_uint16(CONSTRAIN(value, FSR_MIN_VALUE, FSR_MAX_VALUE), FSR_MIN_VALUE, FSR_MAX_VALUE, 0, 127);

        case aftertouch:
        return mapRange_uint16(CONSTRAIN(value, FSR_MIN_VALUE, AFTERTOUCH_MAX_VALUE), FSR_MIN_VALUE, AFTERTOUCH_MAX_VALUE, 0, 127);

        default:
        return 0;
    }
}

bool Analog::getFsrPressed(uint8_t fsrID)
{
    uint8_t arrayIndex = fsrID/8;
    uint8_t fsrIndex = fsrID - 8*arrayIndex;

    return BIT_READ(fsrPressed[arrayIndex], fsrIndex);
}

void Analog::setFsrPressed(uint8_t fsrID, bool state)
{
    uint8_t arrayIndex = fsrID/8;
    uint8_t fsrIndex = fsrID - 8*arrayIndex;

    BIT_WRITE(fsrPressed[arrayIndex], fsrIndex, state);
}

void Analog::checkFSRvalue(uint8_t analogID, uint16_t pressure)
{
    uint8_t calibratedPressure = calibratePressure(pressure, velocity);

    if (calibratedPressure > 0)
    {
        if (!getFsrPressed(analogID))
        {
            //sensor is really pressed
            setFsrPressed(analogID, true);
            uint8_t note = database.read(DB_BLOCK_ANALOG, analogMIDIidSection, analogID);
            midi.sendNoteOn(note, calibratedPressure, database.read(DB_BLOCK_MIDI, midiChannelSection, noteChannel));
            leds.noteToState(note, calibratedPressure, true);
            if (sysEx.configurationEnabled())
            {
                if ((rTimeMs() - getLastCinfoMsgTime(DB_BLOCK_BUTTON)) > COMPONENT_INFO_TIMEOUT)
                {
                    sysEx.startResponse();
                    sysEx.addToResponse(COMPONENT_ID_STRING);
                    sysEx.addToResponse(DB_BLOCK_ANALOG);
                    sysEx.addToResponse(analogID);
                    sysEx.sendResponse();
                    updateCinfoTime(DB_BLOCK_BUTTON);
                }
            }
        }
    }
    else
    {
        if (getFsrPressed(analogID))
        {
            setFsrPressed(analogID, false);
            uint8_t note = database.read(DB_BLOCK_ANALOG, analogMIDIidSection, analogID);
            midi.sendNoteOff(note, 0, database.read(DB_BLOCK_MIDI, midiChannelSection, noteChannel));
            leds.noteToState(note, 0, true);
            if (sysEx.configurationEnabled())
            {
                if ((rTimeMs() - getLastCinfoMsgTime(DB_BLOCK_BUTTON)) > COMPONENT_INFO_TIMEOUT)
                {
                    sysEx.startResponse();
                    sysEx.addToResponse(COMPONENT_ID_STRING);
                    sysEx.addToResponse(DB_BLOCK_ANALOG);
                    sysEx.addToResponse(analogID);
                    sysEx.sendResponse();
                    updateCinfoTime(DB_BLOCK_BUTTON);
                }
            }
        }
    }

    //update values
    lastAnalogueValue[analogID] = pressure;
}
