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

#include "../../../common/Common.h"
#include "Pins.h"
#include "Variables.h"
#include "Hardware.h"

///
/// \brief Hardcoded board version.
/// @{
///

#define HARDWARE_VERSION_MAJOR      1
#define HARDWARE_VERSION_MINOR      0
#define HARDWARE_VERSION_REVISION   0

/// @}

class Board
{
    public:
    Board();
    static void init();

    private:
    static void initUART_MIDI(uint16_t baudRate);
    static void initUSB_MIDI();
    static void initPins();
};