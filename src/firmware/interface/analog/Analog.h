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

#pragma once

#include "../digital/input/buttons/Buttons.h"
#include "../../database/Database.h"
#include "DataTypes.h"
#include "Config.h"

///
/// \brief Analog components handling.
/// \defgroup analog Analog
/// \ingroup interface
/// @{
///

class Analog
{
    public:
    Analog();
    void update();
    void debounceReset(uint16_t index);

    private:
    //variables
    uint8_t     fsrPressed[MAX_NUMBER_OF_ANALOG/8+1];
    uint16_t    lastAnalogueValue[MAX_NUMBER_OF_ANALOG];

    //data processing
    void checkPotentiometerValue(uint8_t analogID, uint16_t value);
    void checkFSRvalue(uint8_t analogID, uint16_t pressure);
    bool fsrPressureStable(uint8_t analogID);
    bool getFsrPressed(uint8_t fsrID);
    void setFsrPressed(uint8_t fsrID, bool state);
    bool getFsrDebounceTimerStarted(uint8_t fsrID);
    void setFsrDebounceTimerStarted(uint8_t fsrID, bool state);
    #ifdef ENABLE_HYSTERESIS
    uint16_t getHysteresisValue(int16_t value);
    #endif
};

extern Analog analog;

/// @}