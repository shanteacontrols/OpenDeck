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

namespace
{
    const char boardName_string[]         = BOARD_STRING;
    const char eventNoteOff_string[]      = "Note Off";
    const char eventNoteOn_string[]       = "Note On";
    const char eventCC_string[]           = "CC";
    const char eventPC_string[]           = "PC";
    const char eventCHAT_string[]         = "CH AT";
    const char eventAT_string[]           = "AT";
    const char eventPB_string[]           = "Pitch Bend";
    const char eventSysExConfig_string[]  = "SysEx Conf";
    const char eventRTclock_string[]      = "RT Clock";
    const char eventRTstart_string[]      = "RT Start";
    const char eventRTcontinue_string[]   = "RT Continue";
    const char eventRTstop_string[]       = "RT Stop";
    const char eventRTasens_string[]      = "RT ASens";
    const char eventRTsysrst_string[]     = "RT SysRst";
    const char eventMMCplay_string[]      = "MMC Play";
    const char eventMMCstop_string[]      = "MMC Stop";
    const char eventMMCpause_string[]     = "MMC Pause";
    const char eventMMCrecordOn_string[]  = "MMC Record On";
    const char eventMMCrecordOff_string[] = "MMC Record Off";
    const char eventNRPN_string[]         = "NRPN";
    const char eventInvalid_string[]      = "";

    const char noteC_string[]      = "C";
    const char noteCSharp_string[] = "C#";
    const char noteD_string[]      = "D";
    const char noteDSharp_string[] = "D#";
    const char noteE_string[]      = "E";
    const char noteF_string[]      = "F";
    const char noteFSharp_string[] = "F#";
    const char noteG_string[]      = "G";
    const char noteGSharp_string[] = "G#";
    const char noteA_string[]      = "A";
    const char noteASharp_string[] = "A#";
    const char noteB_strig[]       = "B";

    const char* noteNameArray[] = {
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
    strcpy(tempBuffer, boardName_string);
    return tempBuffer;
}

const char* Strings::midiMessage(MIDI::messageType_t message)
{
    switch (message)
    {
    case MIDI::messageType_t::noteOff:
    {
        strcpy(tempBuffer, eventNoteOff_string);
    }
    break;

    case MIDI::messageType_t::noteOn:
    {
        strcpy(tempBuffer, eventNoteOn_string);
    }
    break;

    case MIDI::messageType_t::controlChange:
    case MIDI::messageType_t::controlChange14bit:
    {
        strcpy(tempBuffer, eventCC_string);
    }
    break;

    case MIDI::messageType_t::programChange:
    {
        strcpy(tempBuffer, eventPC_string);
    }
    break;

    case MIDI::messageType_t::afterTouchChannel:
    {
        strcpy(tempBuffer, eventCHAT_string);
    }
    break;

    case MIDI::messageType_t::afterTouchPoly:
    {
        strcpy(tempBuffer, eventAT_string);
    }
    break;

    case MIDI::messageType_t::pitchBend:
    {
        strcpy(tempBuffer, eventPB_string);
    }
    break;

    case MIDI::messageType_t::systemExclusive:
    {
        strcpy(tempBuffer, eventSysExConfig_string);
    }
    break;

    case MIDI::messageType_t::sysRealTimeClock:
    {
        strcpy(tempBuffer, eventRTclock_string);
    }
    break;

    case MIDI::messageType_t::sysRealTimeStart:
    {
        strcpy(tempBuffer, eventRTstart_string);
    }
    break;

    case MIDI::messageType_t::sysRealTimeContinue:
    {
        strcpy(tempBuffer, eventRTcontinue_string);
    }
    break;

    case MIDI::messageType_t::sysRealTimeStop:
    {
        strcpy(tempBuffer, eventRTstop_string);
    }
    break;

    case MIDI::messageType_t::sysRealTimeActiveSensing:
    {
        strcpy(tempBuffer, eventRTasens_string);
    }
    break;

    case MIDI::messageType_t::sysRealTimeSystemReset:
    {
        strcpy(tempBuffer, eventRTsysrst_string);
    }
    break;

    case MIDI::messageType_t::mmcPlay:
    {
        strcpy(tempBuffer, eventMMCplay_string);
    }
    break;

    case MIDI::messageType_t::mmcStop:
    {
        strcpy(tempBuffer, eventMMCstop_string);
    }
    break;

    case MIDI::messageType_t::mmcPause:
    {
        strcpy(tempBuffer, eventMMCpause_string);
    }
    break;

    case MIDI::messageType_t::mmcRecordStart:
    {
        strcpy(tempBuffer, eventMMCrecordOn_string);
    }
    break;

    case MIDI::messageType_t::mmcRecordStop:
    {
        strcpy(tempBuffer, eventMMCrecordOff_string);
    }
    break;

    case MIDI::messageType_t::nrpn7bit:
    case MIDI::messageType_t::nrpn14bit:
    {
        strcpy(tempBuffer, eventNRPN_string);
    }
    break;

    default:
    {
        strcpy(tempBuffer, eventInvalid_string);
    }
    break;
    }

    return tempBuffer;
}

const char* Strings::note(MIDI::note_t note)
{
    strcpy(tempBuffer, noteNameArray[static_cast<uint8_t>(note)]);
    return tempBuffer;
}