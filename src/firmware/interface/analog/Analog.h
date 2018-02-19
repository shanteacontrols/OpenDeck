/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

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
    static void update();
    static void debounceReset(uint16_t index);

    private:
    //data processing
    static void checkPotentiometerValue(uint8_t analogID, uint16_t value);
    static void checkFSRvalue(uint8_t analogID, uint16_t pressure);
    static bool fsrPressureStable(uint8_t analogID);
    static bool getFsrPressed(uint8_t fsrID);
    static void setFsrPressed(uint8_t fsrID, bool state);
    static bool getFsrDebounceTimerStarted(uint8_t fsrID);
    static void setFsrDebounceTimerStarted(uint8_t fsrID, bool state);
};

extern Analog analog;

/// @}