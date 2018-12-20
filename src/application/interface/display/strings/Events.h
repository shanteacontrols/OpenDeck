/*

Copyright 2015-2018 Igor Petrovic

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

const char eventPresetChange_string[] PROGMEM = "Preset";

const char eventMIDIin_string[] PROGMEM = "In: ";
const char eventMIDIout_string[] PROGMEM = "Out: ";

//match with messageTypeDisplay_t
PGM_P const eventNameArray[] PROGMEM =
{
    eventNoteOff_string,
    eventNoteOn_string,
    eventCC_string,
    eventPC_string,
    nullptr, //channel aftertouch
    nullptr, //poly aftertouch
    nullptr, //pitch bend
    eventSysExConfig_string,
    nullptr, //sys common - Time Code Quarter Frame
    nullptr, //sys common - Song Position Pointer
    nullptr, //sys common - Song Select
    nullptr, //sys common - Tune Request
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
    eventNRPN_string,
    eventPresetChange_string
};
