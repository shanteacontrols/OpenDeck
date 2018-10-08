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

const char arduinoLeonardo_string[] PROGMEM = "Arduino Leo";
const char arduinoMega_string[] PROGMEM = "Arduino Mega";
const char arduinoProMicro_string[] PROGMEM = "Arduino PM";
const char arduinoUno_string[] PROGMEM = "Arduino Uno";
const char teensy2pp_string[] PROGMEM = "Teensy++ 2.0";
const char kodama_string[] PROGMEM = "Kodama";
const char tannin_string[] PROGMEM = "Tannin";

//match with list from BoardIDs.mk
PGM_P const boardNameArray[] PROGMEM =
{
    nullptr, //opendeck board, no string
    arduinoLeonardo_string,
    arduinoMega_string,
    arduinoProMicro_string,
    arduinoUno_string,
    teensy2pp_string,
    kodama_string,
    tannin_string,
    nullptr //xu2
};
