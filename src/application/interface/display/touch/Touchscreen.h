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

#include "DataTypes.h"

///
/// \brief Touchscreen control.
/// \defgroup interfaceLCDTouch Touchscreen
/// \ingroup interfaceLCD
/// @{

class Touchscreen
{
    public:
    Touchscreen();
    static bool init(ts_t touchscreenType);
    static void update();
    static void setPage(uint8_t pageID);
    static uint8_t getPage();
    static void setBrightness(backlightType_t type, int8_t value);

    private:
    static void process(uint8_t buttonID, bool buttonState);
};

///
/// \brief External definition of Touchscreen class instance.
///
extern Touchscreen touchscreen;

/// @}
