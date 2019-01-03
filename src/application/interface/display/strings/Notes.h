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
