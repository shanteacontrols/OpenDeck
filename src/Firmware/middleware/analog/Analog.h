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

#pragma once

#include "../../board/Board.h"
#include "../buttons/Buttons.h"
#include "../../eeprom/Database.h"

#define NUMBER_OF_SAMPLES 3 //do not change

enum ccLimitType_t
{
    ccLimitLow,
    ccLimitHigh
};

enum analogType_t
{
    potentiometer,
    fsr,
    ldr,
    ANALOG_TYPES
};

class Analog : Board
{
    public:
    Analog();
    void update();
    void debounceReset(uint16_t index);

    private:

    //variables
    uint8_t analogDebounceCounter,
            fsrPressed[MAX_NUMBER_OF_ANALOG/8+1],
            fsrLastAfterTouchValue[MAX_NUMBER_OF_ANALOG];

    int16_t analogSample[MAX_NUMBER_OF_ANALOG][NUMBER_OF_SAMPLES],
            lastAnalogueValue[MAX_NUMBER_OF_ANALOG];

    //data processing
    void checkPotentiometerValue(uint8_t analogID, int16_t tempValue);
    void checkFSRvalue(uint8_t analogID, int16_t pressure);
    bool fsrPressureStable(uint8_t analogID);
    bool getFsrPressed(uint8_t fsrID);
    void setFsrPressed(uint8_t fsrID, bool state);
    bool getFsrDebounceTimerStarted(uint8_t fsrID);
    void setFsrDebounceTimerStarted(uint8_t fsrID, bool state);
    int16_t getMedianValue(uint8_t analogID);
    void addAnalogSamples();
    bool analogValuesSampled();
    inline uint8_t mapAnalog_uint8(uint8_t x, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max) {

        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

    };
};

extern Analog analog;
