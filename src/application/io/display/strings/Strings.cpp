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
    const char boardName_string[] PROGMEM         = BOARD_STRING;
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
    const char eventInvalid_string[] PROGMEM      = "";

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

const char* Strings::midiMessage(MIDI::messageType_t message)
{
    switch (message)
    {
    case MIDI::messageType_t::noteOff:
    {
        strcpy_P(tempBuffer, READ_PROGMEM_WORD(eventNoteOff_string));
    }
    break;

    case MIDI::messageType_t::noteOn:
    {
        strcpy_P(tempBuffer, READ_PROGMEM_WORD(eventNoteOn_string));
    }
    break;

    case MIDI::messageType_t::controlChange:
    case MIDI::messageType_t::controlChange14bit:
    {
        strcpy_P(tempBuffer, READ_PROGMEM_WORD(eventCC_string));
    }
    break;

    case MIDI::messageType_t::programChange:
    {
        strcpy_P(tempBuffer, READ_PROGMEM_WORD(eventPC_string));
    }
    break;

    case MIDI::messageType_t::afterTouchChannel:
    {
        strcpy_P(tempBuffer, READ_PROGMEM_WORD(eventCHAT_string));
    }
    break;

    case MIDI::messageType_t::afterTouchPoly:
    {
        strcpy_P(tempBuffer, READ_PROGMEM_WORD(eventAT_string));
    }
    break;

    case MIDI::messageType_t::pitchBend:
    {
        strcpy_P(tempBuffer, READ_PROGMEM_WORD(eventPB_string));
    }
    break;

    case MIDI::messageType_t::systemExclusive:
    {
        strcpy_P(tempBuffer, READ_PROGMEM_WORD(eventSysExConfig_string));
    }
    break;

    case MIDI::messageType_t::sysRealTimeClock:
    {
        strcpy_P(tempBuffer, READ_PROGMEM_WORD(eventRTclock_string));
    }
    break;

    case MIDI::messageType_t::sysRealTimeStart:
    {
        strcpy_P(tempBuffer, READ_PROGMEM_WORD(eventRTstart_string));
    }
    break;

    case MIDI::messageType_t::sysRealTimeContinue:
    {
        strcpy_P(tempBuffer, READ_PROGMEM_WORD(eventRTcontinue_string));
    }
    break;

    case MIDI::messageType_t::sysRealTimeStop:
    {
        strcpy_P(tempBuffer, READ_PROGMEM_WORD(eventRTstop_string));
    }
    break;

    case MIDI::messageType_t::sysRealTimeActiveSensing:
    {
        strcpy_P(tempBuffer, READ_PROGMEM_WORD(eventRTasens_string));
    }
    break;

    case MIDI::messageType_t::sysRealTimeSystemReset:
    {
        strcpy_P(tempBuffer, READ_PROGMEM_WORD(eventRTsysrst_string));
    }
    break;

    case MIDI::messageType_t::mmcPlay:
    {
        strcpy_P(tempBuffer, READ_PROGMEM_WORD(eventMMCplay_string));
    }
    break;

    case MIDI::messageType_t::mmcStop:
    {
        strcpy_P(tempBuffer, READ_PROGMEM_WORD(eventMMCstop_string));
    }
    break;

    case MIDI::messageType_t::mmcPause:
    {
        strcpy_P(tempBuffer, READ_PROGMEM_WORD(eventMMCpause_string));
    }
    break;

    case MIDI::messageType_t::mmcRecordStart:
    {
        strcpy_P(tempBuffer, READ_PROGMEM_WORD(eventMMCrecordOn_string));
    }
    break;

    case MIDI::messageType_t::mmcRecordStop:
    {
        strcpy_P(tempBuffer, READ_PROGMEM_WORD(eventMMCrecordOff_string));
    }
    break;

    case MIDI::messageType_t::nrpn7bit:
    case MIDI::messageType_t::nrpn14bit:
    {
        strcpy_P(tempBuffer, READ_PROGMEM_WORD(eventNRPN_string));
    }
    break;

    default:
    {
        strcpy_P(tempBuffer, READ_PROGMEM_WORD(eventInvalid_string));
    }
    break;
    }

    return tempBuffer;
}

const char* Strings::note(MIDI::note_t note)
{
    strcpy_P(tempBuffer, READ_PROGMEM_ARRAY(noteNameArray[static_cast<uint8_t>(note)]));
    return tempBuffer;
}