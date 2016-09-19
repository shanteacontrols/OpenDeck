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

#ifndef INIT_H_
#define INIT_H_

#include "../eeprom/Configuration.h"
#include "../sysex/SysEx.h"
#include "../version/Firmware.h"
#include "../version/Hardware.h"
#include "../core/Core.h"
#include "../board/Board.h"
#include "../middleware/Middleware.h"

void globalInit();

#endif