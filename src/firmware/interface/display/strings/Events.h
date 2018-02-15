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

const char eventCC_string[] PROGMEM = "CC";
const char eventPC_string[] PROGMEM = "PC";
const char eventMMCplay_string[] PROGMEM = "MMC Play";
const char eventMMCstop_string[] PROGMEM = "MMC Stop";
const char eventMMCpause_string[] PROGMEM = "MMC Pause";
const char eventMMCrecord_string[] PROGMEM = "MMC Record";

const char eventRTclock_string[] PROGMEM = "RealTime Clock";
const char eventRTstart_string[] PROGMEM = "RealTime Start";
const char eventRTcontinue_string[] PROGMEM = "RealTime Continue";
const char eventRTasens_string[] PROGMEM = "RealTime ASens";
const char eventRTsysrst_string[] PROGMEM = "RealTime SysRst";

const char eventMIDIin_string[] PROGMEM = "In";
const char eventMIDIout_string[] PROGMEM = "Out";
