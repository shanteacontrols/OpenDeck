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

#include "Analog.h"

//use 1k resistor when connecting FSR between signal and ground

#define FSR_MIN_VALUE                       40
#define FSR_MAX_VALUE                       340

#define AFTERTOUCH_MAX_VALUE                600
#define AFTERTOUCH_SEND_TIMEOUT_IGNORE      25       //ignore aftertouch reading change below this timeout
#define AFTERTOUCH_SEND_TIMEOUT_STEP        2
#define AFTERTOUCH_SEND_TIMEOUT             100

enum pressureType_t {

    velocity,
    aftertouch

};

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

inline int16_t mapAnalog_int16(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max) {

    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

};

inline int16_t calibratePressure(int16_t value, pressureType_t type)  {

    switch(type)    {

        case velocity:
        return mapAnalog_int16(constrain(value, FSR_MIN_VALUE, FSR_MAX_VALUE), FSR_MIN_VALUE, FSR_MAX_VALUE, 0, 127);

        case aftertouch:
        return mapAnalog_int16(constrain(value, FSR_MIN_VALUE, AFTERTOUCH_MAX_VALUE), FSR_MIN_VALUE, AFTERTOUCH_MAX_VALUE, 0, 127);

        default:
        return 0;

    }

}

bool Analog::getFsrPressed(uint8_t fsrID)   {

    uint8_t arrayIndex = fsrID/8;
    uint8_t fsrIndex = fsrID - 8*arrayIndex;

    return bitRead(fsrPressed[arrayIndex], fsrIndex);

}

void Analog::setFsrPressed(uint8_t fsrID, bool state)   {

    uint8_t arrayIndex = fsrID/8;
    uint8_t fsrIndex = fsrID - 8*arrayIndex;

    bitWrite(fsrPressed[arrayIndex], fsrIndex, state);

}

void Analog::checkFSRvalue(uint8_t analogID, int16_t pressure)  {

    uint8_t calibratedPressure = calibratePressure(pressure, velocity);

    lastAnalogueValue[analogID] += calibratedPressure;

    calibratedPressure = lastAnalogueValue[analogID];
    lastAnalogueValue[analogID] = 0;

    bool pressDetected = (calibratedPressure > 0);

    switch (pressDetected)    {

        case true:

        if (!getFsrPressed(analogID)) {

            //sensor is really pressed
            setFsrPressed(analogID, true);
            midi.sendNoteOn(database.readParameter(CONF_BLOCK_ANALOG, analogMIDIidSection, analogID), calibratedPressure);
            //if (sysEx.configurationEnabled())
                //sysEx.sendComponentID(CONF_BLOCK_ANALOG, analogID);

        }
        break;

        case false:
        if (getFsrPressed(analogID))  {

            setFsrPressed(analogID, false);
            midi.sendNoteOff(database.readParameter(CONF_BLOCK_ANALOG, analogMIDIidSection, analogID), 0);
            //if (sysEx.configurationEnabled())
                //sysEx.sendComponentID(CONF_BLOCK_ANALOG, analogID);

        }
        break;

    }

}
