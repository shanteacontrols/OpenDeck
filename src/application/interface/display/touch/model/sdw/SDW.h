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

#include "../../DataTypes.h"    //general data types
#include "DataTypes.h"          //sdw-specific data types

void sdw_init();
void sdw_sendByte(uint8_t value, messageByteType_t messageByteType);
bool sdw_parse();
bool sdw_update();
void sdw_setImage(uint8_t imageID);
void sdw_setIcon(uint16_t x, uint16_t y, uint16_t iconID);
void sdw_setBrightness(backlightType_t type, int8_t value);