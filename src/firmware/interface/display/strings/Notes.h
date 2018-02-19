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

const char noteC_string[] PROGMEM = "C";
const char noteCSharp_string[] PROGMEM = "C#";
const char noteD_string[] PROGMEM = "D";
const char noteDSharp_string[] PROGMEM = "D#";
const char noteE_string[] PROGMEM = "E";
const char noteF_string[] PROGMEM = "F";
const char noteFSharp_string[] PROGMEM = "F#";
const char noteG_string[] PROGMEM = "G";
const char noteGSharp_string[] PROGMEM = "G#";
const char noteA_string[] PROGMEM = "A";
const char noteASharp_string[] PROGMEM = "A#";
const char noteB_strig[] PROGMEM = "B";

const char velocity_string[] PROGMEM = "v: ";
const char aftertouch_string[] PROGMEM = "at: ";

PGM_P const noteNameArray[] PROGMEM =
{
    noteC_string,
    noteCSharp_string,
    noteD_string,
    noteDSharp_string,
    noteE_string,
    noteF_string,
    noteFSharp_string,
    noteG_string,
    noteGSharp_string,
    noteA_string,
    noteASharp_string,
    noteB_strig
};
