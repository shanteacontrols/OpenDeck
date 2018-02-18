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

const char eventNoteOff_string[] PROGMEM = "Note Off";
const char eventNoteOn_string[] PROGMEM = "Note On";
const char eventCC_string[] PROGMEM = "CC";
const char eventPC_string[] PROGMEM = "PC";

const char eventSysExConfig_string[] PROGMEM = "SysEx Conf";

const char eventRTclock_string[] PROGMEM = "RT Clock";
const char eventRTstart_string[] PROGMEM = "RT Start";
const char eventRTcontinue_string[] PROGMEM = "RT Continue";
const char eventRTstop_string[] PROGMEM = "RT Stop";
const char eventRTasens_string[] PROGMEM = "RT ASens";
const char eventRTsysrst_string[] PROGMEM = "RT SysRst";

const char eventMMCplay_string[] PROGMEM = "MMC Play";
const char eventMMCstop_string[] PROGMEM = "MMC Stop";
const char eventMMCpause_string[] PROGMEM = "MMC Pause";
const char eventMMCrecordOn_string[] PROGMEM = "MMC Record On";
const char eventMMCrecordOff_string[] PROGMEM = "MMC Record Off";

const char eventNRPN_string[] PROGMEM = "NRPN";

const char eventMIDIin_string[] PROGMEM = "In: ";
const char eventMIDIout_string[] PROGMEM = "Out: ";

//match with midiMessageTypeDisplay_t
PGM_P const eventNameArray[] PROGMEM =
{
    eventNoteOff_string,
    eventNoteOn_string,
    eventCC_string,
    eventPC_string,
    NULL, //channel aftertouch
    NULL, //poly aftertouch
    NULL, //pitch bend
    eventSysExConfig_string,
    NULL, //sys common - Time Code Quarter Frame
    NULL, //sys common - Song Position Pointer
    NULL, //sys common - Song Select
    NULL, //sys common - Tune Request
    eventRTclock_string,
    eventRTstart_string,
    eventRTcontinue_string,
    eventRTstop_string,
    eventRTasens_string,
    eventRTsysrst_string,
    eventMMCplay_string,
    eventMMCstop_string,
    eventMMCpause_string,
    eventMMCrecordOn_string,
    eventMMCrecordOff_string,
    eventNRPN_string
};
