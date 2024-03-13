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

#ifdef PROJECT_TARGET_SUPPORT_DISPLAY

#include "Strings.h"
#include <string.h>
#include <stdio.h>

namespace
{
    const char TARGET_NAME_STRING[]          = PROJECT_TARGET_NAME;
    const char EVENT_NOTE_OFF_STRING[]       = "Note Off";
    const char EVENT_NOTE_ON_STRING[]        = "Note On";
    const char EVENT_CC_STRING[]             = "CC";
    const char EVENT_PC_STRING[]             = "PC";
    const char EVENT_CH_AT_STRING[]          = "CH AT";
    const char EVENT_AT_STRING[]             = "AT";
    const char EVENT_PB_STRING[]             = "Pitch Bend";
    const char EVENT_SYS_EX_CONFIG_STRING[]  = "SysEx Conf";
    const char EVENT_RT_CLOCK_STRING[]       = "RT Clock";
    const char EVENT_RT_START_STRING[]       = "RT Start";
    const char EVENT_RT_CONTINUE_STRING[]    = "RT Continue";
    const char EVENT_RT_STOP_STRING[]        = "RT Stop";
    const char EVENT_RT_ASENS_STRING[]       = "RT ASens";
    const char EVENT_RT_SYSRST_STRING[]      = "RT SysRst";
    const char EVENT_MMC_PLAY_STRING[]       = "MMC Play";
    const char EVENT_MMC_STOP_STRING[]       = "MMC Stop";
    const char EVENT_MMC_PAUSE_STRING[]      = "MMC Pause";
    const char EVENT_MMC_RECORD_ON_STRING[]  = "MMC Rec On";
    const char EVENT_MMC_RECORD_OFF_STRING[] = "MMC Rec Off";
    const char EVENT_NRPN_STRING[]           = "NRPN";
    const char EVENT_INVALID_STRING[]        = "";

    const char NOTE_C_STRING[]       = "C";
    const char NOTE_C_SHARP_STRING[] = "C#";
    const char NOTE_D_STRING[]       = "D";
    const char NOTE_D_SHARP_STRING[] = "D#";
    const char NOTE_E_STRING[]       = "E";
    const char NOTE_F_STRING[]       = "F";
    const char NOTE_F_SHARP_STRING[] = "F#";
    const char NOTE_G_STRING[]       = "G";
    const char NOTE_G_SHARP_STRING[] = "G#";
    const char NOTE_A_STRING[]       = "A";
    const char NOTE_A_SHARP_STRING[] = "A#";
    const char NOTE_B_STRIG[]        = "B";

    const char* const NOTE_NAME_ARRAY[] = {
        NOTE_C_STRING,
        NOTE_C_SHARP_STRING,
        NOTE_D_STRING,
        NOTE_D_SHARP_STRING,
        NOTE_E_STRING,
        NOTE_F_STRING,
        NOTE_F_SHARP_STRING,
        NOTE_G_STRING,
        NOTE_G_SHARP_STRING,
        NOTE_A_STRING,
        NOTE_A_SHARP_STRING,
        NOTE_B_STRIG
    };

    static constexpr size_t BUFFER_SIZE = 50;
    char                    tempBuffer[BUFFER_SIZE];
}    // namespace

const char* Strings::target()
{
    strncpy(tempBuffer, TARGET_NAME_STRING, BUFFER_SIZE);
    return tempBuffer;
}

const char* Strings::midiMessage(MIDI::messageType_t message)
{
    switch (message)
    {
    case MIDI::messageType_t::NOTE_OFF:
    {
        strncpy(tempBuffer, EVENT_NOTE_OFF_STRING, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::NOTE_ON:
    {
        strncpy(tempBuffer, EVENT_NOTE_ON_STRING, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::CONTROL_CHANGE:
    case MIDI::messageType_t::CONTROL_CHANGE_14BIT:
    {
        strncpy(tempBuffer, EVENT_CC_STRING, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::PROGRAM_CHANGE:
    {
        strncpy(tempBuffer, EVENT_PC_STRING, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::AFTER_TOUCH_CHANNEL:
    {
        strncpy(tempBuffer, EVENT_CH_AT_STRING, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::AFTER_TOUCH_POLY:
    {
        strncpy(tempBuffer, EVENT_AT_STRING, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::PITCH_BEND:
    {
        strncpy(tempBuffer, EVENT_PB_STRING, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::SYS_EX:
    {
        strncpy(tempBuffer, EVENT_SYS_EX_CONFIG_STRING, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::SYS_REAL_TIME_CLOCK:
    {
        strncpy(tempBuffer, EVENT_RT_CLOCK_STRING, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::SYS_REAL_TIME_START:
    {
        strncpy(tempBuffer, EVENT_RT_START_STRING, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::SYS_REAL_TIME_CONTINUE:
    {
        strncpy(tempBuffer, EVENT_RT_CONTINUE_STRING, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::SYS_REAL_TIME_STOP:
    {
        strncpy(tempBuffer, EVENT_RT_STOP_STRING, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::SYS_REAL_TIME_ACTIVE_SENSING:
    {
        strncpy(tempBuffer, EVENT_RT_ASENS_STRING, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::SYS_REAL_TIME_SYSTEM_RESET:
    {
        strncpy(tempBuffer, EVENT_RT_SYSRST_STRING, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::MMC_PLAY:
    {
        strncpy(tempBuffer, EVENT_MMC_PLAY_STRING, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::MMC_STOP:
    {
        strncpy(tempBuffer, EVENT_MMC_STOP_STRING, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::MMC_PAUSE:
    {
        strncpy(tempBuffer, EVENT_MMC_PAUSE_STRING, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::MMC_RECORD_START:
    {
        strncpy(tempBuffer, EVENT_MMC_RECORD_ON_STRING, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::MMC_RECORD_STOP:
    {
        strncpy(tempBuffer, EVENT_MMC_RECORD_OFF_STRING, BUFFER_SIZE);
    }
    break;

    case MIDI::messageType_t::NRPN_7BIT:
    case MIDI::messageType_t::NRPN_14BIT:
    {
        strncpy(tempBuffer, EVENT_NRPN_STRING, BUFFER_SIZE);
    }
    break;

    default:
    {
        strncpy(tempBuffer, EVENT_INVALID_STRING, BUFFER_SIZE);
    }
    break;
    }

    return tempBuffer;
}

const char* Strings::note(MIDI::note_t note)
{
    strncpy(tempBuffer, NOTE_NAME_ARRAY[static_cast<uint8_t>(note)], BUFFER_SIZE);
    return tempBuffer;
}

#endif