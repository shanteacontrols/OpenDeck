/*

Copyright 2015-2019 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

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
