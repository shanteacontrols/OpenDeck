/*

Copyright 2015-2022 Igor Petrovic

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

#ifdef I2C_SUPPORTED

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

    static constexpr size_t BUFFER_SIZE = 50;
    char                    tempBuffer[BUFFER_SIZE];
}    // namespace

const char* Strings::board()
{
    strncpy(tempBuffer, boardName_string, BUFFER_SIZE);
    return tempBuffer;
}

const char* Strings::midiMessage(MIDI::messageType_t message)
{
    switch (message)
    {
    case MIDI::messageType_t::NOTE_OFF:
    {
        strncpy(tempBuffer, eventNoteOff_string, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::NOTE_ON:
    {
        strncpy(tempBuffer, eventNoteOn_string, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::CONTROL_CHANGE:
    case MIDI::messageType_t::CONTROL_CHANGE_14BIT:
    {
        strncpy(tempBuffer, eventCC_string, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::PROGRAM_CHANGE:
    {
        strncpy(tempBuffer, eventPC_string, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::AFTER_TOUCH_CHANNEL:
    {
        strncpy(tempBuffer, eventCHAT_string, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::AFTER_TOUCH_POLY:
    {
        strncpy(tempBuffer, eventAT_string, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::PITCH_BEND:
    {
        strncpy(tempBuffer, eventPB_string, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::SYS_EX:
    {
        strncpy(tempBuffer, eventSysExConfig_string, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::SYS_REAL_TIME_CLOCK:
    {
        strncpy(tempBuffer, eventRTclock_string, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::SYS_REAL_TIME_START:
    {
        strncpy(tempBuffer, eventRTstart_string, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::SYS_REAL_TIME_CONTINUE:
    {
        strncpy(tempBuffer, eventRTcontinue_string, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::SYS_REAL_TIME_STOP:
    {
        strncpy(tempBuffer, eventRTstop_string, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::SYS_REAL_TIME_ACTIVE_SENSING:
    {
        strncpy(tempBuffer, eventRTasens_string, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::SYS_REAL_TIME_SYSTEM_RESET:
    {
        strncpy(tempBuffer, eventRTsysrst_string, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::MMC_PLAY:
    {
        strncpy(tempBuffer, eventMMCplay_string, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::MMC_STOP:
    {
        strncpy(tempBuffer, eventMMCstop_string, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::MMC_PAUSE:
    {
        strncpy(tempBuffer, eventMMCpause_string, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::MMC_RECORD_START:
    {
        strncpy(tempBuffer, eventMMCrecordOn_string, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::MMC_RECORD_STOP:
    {
        strncpy(tempBuffer, eventMMCrecordOff_string, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::NRPN_7BIT:
    case MIDI::messageType_t::NRPN_14BIT:
    {
        strncpy(tempBuffer, eventNRPN_string, BUFFER_SIZE);
    }
    break;

    default:
    {
        strncpy(tempBuffer, eventInvalid_string, BUFFER_SIZE);
    }
    break;
    }

    return tempBuffer;
}

const char* Strings::note(MIDI::note_t note)
{
    strncpy(tempBuffer, noteNameArray[static_cast<uint8_t>(note)], BUFFER_SIZE);
    return tempBuffer;
}

#endif