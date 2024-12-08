/*

Copyright Igor Petrovic

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

#include "application/protocol/midi/midi.h"

namespace io::i2c::display
{
    class Strings
    {
        public:
        Strings() = default;

        static constexpr auto TARGET_NAME_STRING          = PROJECT_TARGET_NAME;
        static constexpr auto IN_EVENT_STRING             = "<";
        static constexpr auto OUT_EVENT_STRING            = ">";
        static constexpr auto NOTE_C_STRING               = "C";
        static constexpr auto NOTE_C_SHARP_STRING         = "C#";
        static constexpr auto NOTE_D_STRING               = "D";
        static constexpr auto NOTE_D_SHARP_STRING         = "D#";
        static constexpr auto NOTE_E_STRING               = "E";
        static constexpr auto NOTE_F_STRING               = "F";
        static constexpr auto NOTE_F_SHARP_STRING         = "F#";
        static constexpr auto NOTE_G_STRING               = "G";
        static constexpr auto NOTE_G_SHARP_STRING         = "G#";
        static constexpr auto NOTE_A_STRING               = "A";
        static constexpr auto NOTE_A_SHARP_STRING         = "A#";
        static constexpr auto NOTE_B_STRIG                = "B";
        static constexpr auto EVENT_NOTE_OFF_STRING       = "Note Off";
        static constexpr auto EVENT_NOTE_ON_STRING        = "Note On";
        static constexpr auto EVENT_CC_STRING             = "CC";
        static constexpr auto EVENT_PC_STRING             = "PC";
        static constexpr auto EVENT_CH_AT_STRING          = "CH AT";
        static constexpr auto EVENT_AT_STRING             = "AT";
        static constexpr auto EVENT_PB_STRING             = "Pitch Bend";
        static constexpr auto EVENT_SYS_EX_CONFIG_STRING  = "SysEx Conf";
        static constexpr auto EVENT_RT_CLOCK_STRING       = "RT Clock";
        static constexpr auto EVENT_RT_START_STRING       = "RT Start";
        static constexpr auto EVENT_RT_CONTINUE_STRING    = "RT Continue";
        static constexpr auto EVENT_RT_STOP_STRING        = "RT Stop";
        static constexpr auto EVENT_RT_ASENS_STRING       = "RT ASens";
        static constexpr auto EVENT_RT_SYSRST_STRING      = "RT SysRst";
        static constexpr auto EVENT_MMC_PLAY_STRING       = "MMC Play";
        static constexpr auto EVENT_MMC_STOP_STRING       = "MMC Stop";
        static constexpr auto EVENT_MMC_PAUSE_STRING      = "MMC Pause";
        static constexpr auto EVENT_MMC_RECORD_ON_STRING  = "MMC Rec On";
        static constexpr auto EVENT_MMC_RECORD_OFF_STRING = "MMC Rec Off";
        static constexpr auto EVENT_NRPN_STRING           = "NRPN";
        static constexpr auto EVENT_INVALID_STRING        = "";

        static constexpr const char* MIDI_MESSAGE(protocol::midi::messageType_t message)
        {
            switch (message)
            {
            case protocol::midi::messageType_t::NOTE_OFF:
            {
                return EVENT_NOTE_OFF_STRING;
            }
            break;

            case protocol::midi::messageType_t::NOTE_ON:
            {
                return EVENT_NOTE_ON_STRING;
            }
            break;

            case protocol::midi::messageType_t::CONTROL_CHANGE:
            case protocol::midi::messageType_t::CONTROL_CHANGE_14BIT:
            {
                return EVENT_CC_STRING;
            }
            break;

            case protocol::midi::messageType_t::PROGRAM_CHANGE:
            {
                return EVENT_PC_STRING;
            }
            break;

            case protocol::midi::messageType_t::AFTER_TOUCH_CHANNEL:
            {
                return EVENT_CH_AT_STRING;
            }
            break;

            case protocol::midi::messageType_t::AFTER_TOUCH_POLY:
            {
                return EVENT_AT_STRING;
            }
            break;

            case protocol::midi::messageType_t::PITCH_BEND:
            {
                return EVENT_PB_STRING;
            }
            break;

            case protocol::midi::messageType_t::SYS_EX:
            {
                return EVENT_SYS_EX_CONFIG_STRING;
            }
            break;

            case protocol::midi::messageType_t::SYS_REAL_TIME_CLOCK:
            {
                return EVENT_RT_CLOCK_STRING;
            }
            break;

            case protocol::midi::messageType_t::SYS_REAL_TIME_START:
            {
                return EVENT_RT_START_STRING;
            }
            break;

            case protocol::midi::messageType_t::SYS_REAL_TIME_CONTINUE:
            {
                return EVENT_RT_CONTINUE_STRING;
            }
            break;

            case protocol::midi::messageType_t::SYS_REAL_TIME_STOP:
            {
                return EVENT_RT_STOP_STRING;
            }
            break;

            case protocol::midi::messageType_t::SYS_REAL_TIME_ACTIVE_SENSING:
            {
                return EVENT_RT_ASENS_STRING;
            }
            break;

            case protocol::midi::messageType_t::SYS_REAL_TIME_SYSTEM_RESET:
            {
                return EVENT_RT_SYSRST_STRING;
            }
            break;

            case protocol::midi::messageType_t::MMC_PLAY:
            {
                return EVENT_MMC_PLAY_STRING;
            }
            break;

            case protocol::midi::messageType_t::MMC_STOP:
            {
                return EVENT_MMC_STOP_STRING;
            }
            break;

            case protocol::midi::messageType_t::MMC_PAUSE:
            {
                return EVENT_MMC_PAUSE_STRING;
            }
            break;

            case protocol::midi::messageType_t::MMC_RECORD_START:
            {
                return EVENT_MMC_RECORD_ON_STRING;
            }
            break;

            case protocol::midi::messageType_t::MMC_RECORD_STOP:
            {
                return EVENT_MMC_RECORD_OFF_STRING;
            }
            break;

            case protocol::midi::messageType_t::NRPN_7BIT:
            case protocol::midi::messageType_t::NRPN_14BIT:
            {
                return EVENT_NRPN_STRING;
            }
            break;

            default:
            {
                return EVENT_INVALID_STRING;
            }
            break;
            }
        }

        static constexpr const char* NOTE(protocol::midi::note_t note)
        {
            return NOTE_NAME_ARRAY[static_cast<uint8_t>(note)];
        }

        private:
        static constexpr const char* NOTE_NAME_ARRAY[] = {
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
    };
}    // namespace io::i2c::display
