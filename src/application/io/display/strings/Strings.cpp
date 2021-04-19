/*

Copyright 2015-2021 Igor Petrovic

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

#include "Strings.h"
#include <string.h>
#include <stdio.h>
#include "core/src/general/Helpers.h"

namespace
{
    const char boardName_string[] PROGMEM         = OD_BOARD;
    const char eventNoteOff_string[] PROGMEM      = "Note Off";
    const char eventNoteOn_string[] PROGMEM       = "Note On";
    const char eventCC_string[] PROGMEM           = "CC";
    const char eventPC_string[] PROGMEM           = "PC";
    const char eventCHAT_string[] PROGMEM         = "CH AT";
    const char eventAT_string[] PROGMEM           = "AT";
    const char eventPB_string[] PROGMEM           = "Pitch Bend";
    const char eventSysExConfig_string[] PROGMEM  = "SysEx Conf";
    const char eventRTclock_string[] PROGMEM      = "RT Clock";
    const char eventRTstart_string[] PROGMEM      = "RT Start";
    const char eventRTcontinue_string[] PROGMEM   = "RT Continue";
    const char eventRTstop_string[] PROGMEM       = "RT Stop";
    const char eventRTasens_string[] PROGMEM      = "RT ASens";
    const char eventRTsysrst_string[] PROGMEM     = "RT SysRst";
    const char eventMMCplay_string[] PROGMEM      = "MMC Play";
    const char eventMMCstop_string[] PROGMEM      = "MMC Stop";
    const char eventMMCpause_string[] PROGMEM     = "MMC Pause";
    const char eventMMCrecordOn_string[] PROGMEM  = "MMC Record On";
    const char eventMMCrecordOff_string[] PROGMEM = "MMC Record Off";
    const char eventNRPN_string[] PROGMEM         = "NRPN";
    const char eventPresetChange_string[] PROGMEM = "Preset";

    const char noteC_string[] PROGMEM      = "C";
    const char noteCSharp_string[] PROGMEM = "C#";
    const char noteD_string[] PROGMEM      = "D";
    const char noteDSharp_string[] PROGMEM = "D#";
    const char noteE_string[] PROGMEM      = "E";
    const char noteF_string[] PROGMEM      = "F";
    const char noteFSharp_string[] PROGMEM = "F#";
    const char noteG_string[] PROGMEM      = "G";
    const char noteGSharp_string[] PROGMEM = "G#";
    const char noteA_string[] PROGMEM      = "A";
    const char noteASharp_string[] PROGMEM = "A#";
    const char noteB_strig[] PROGMEM       = "B";

    //match with messageTypeDisplay_t
    STRING_PROGMEM_ARRAY(eventNameArray) = {
        eventNoteOff_string,
        eventNoteOn_string,
        eventCC_string,
        eventPC_string,
        eventCHAT_string,
        eventAT_string,
        eventPB_string,
        eventSysExConfig_string,
        nullptr,    //sys common - Time Code Quarter Frame
        nullptr,    //sys common - Song Position Pointer
        nullptr,    //sys common - Song Select
        nullptr,    //sys common - Tune Request
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

    STRING_PROGMEM_ARRAY(noteNameArray) = {
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

    char tempBuffer[50];
}    // namespace

const char* Strings::board()
{
    strcpy_P(tempBuffer, boardName_string);
    return tempBuffer;
}

const char* Strings::midiMessage(IO::Display::event_t event)
{
    strcpy_P(tempBuffer, READ_PROGMEM_ARRAY(eventNameArray[static_cast<uint8_t>(event)]));
    return tempBuffer;
}

const char* Strings::note(MIDI::note_t note)
{
    strcpy_P(tempBuffer, READ_PROGMEM_ARRAY(noteNameArray[static_cast<uint8_t>(note)]));
    return tempBuffer;
}