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

#include "database/Database.h"
#include "sysex/SysEx.h"
#include "version/Version.h"
#include "board/Board.h"
#include "interface/Interface.h"

#define FIRMWARE_VERSION_STRING         0x56
#define HARDWARE_VERSION_STRING         0x42
#define REBOOT_APP_STRING               0x7F
#define REBOOT_BTLDR_STRING             0x55
#define FACTORY_RESET_STRING            0x44
#define COMPONENT_ID_STRING             0x49
#define MAX_COMPONENTS_STRING           0x4D

#define COMPONENT_INFO_TIMEOUT          500 //ms

int main();
void globalInit();
sysExParameter_t onGet(uint8_t block, uint8_t section, uint16_t index);
bool onSet(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t newValue);
bool onCustom(uint8_t value);
void updateCinfoTime(uint8_t block);
uint32_t getLastCinfoMsgTime(uint8_t block);
