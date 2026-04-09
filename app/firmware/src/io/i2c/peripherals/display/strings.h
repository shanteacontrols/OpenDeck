/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "protocol/midi/midi.h"

namespace io::i2c::display
{
    /**
     * @brief Collection of string constants used by the display subsystem.
     */
    class Strings
    {
        public:
        Strings() = default;

        /**
         * @brief String table used by the display subsystem for board labels, activity indicators,
         * note names, and human-readable MIDI event names.
         */
        static constexpr auto TARGET_NAME_STRING          = CONFIG_BOARD;
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

        /**
         * @brief Returns the display label for one MIDI message type.
         *
         * @param message MIDI message type to stringify.
         *
         * @return Display string matching the message type.
         */
        static constexpr const char* midi_message(protocol::midi::MessageType message)
        {
            switch (message)
            {
            case protocol::midi::MessageType::NoteOff:
            {
                return EVENT_NOTE_OFF_STRING;
            }
            break;

            case protocol::midi::MessageType::NoteOn:
            {
                return EVENT_NOTE_ON_STRING;
            }
            break;

            case protocol::midi::MessageType::ControlChange:
            case protocol::midi::MessageType::ControlChange14Bit:
            {
                return EVENT_CC_STRING;
            }
            break;

            case protocol::midi::MessageType::ProgramChange:
            {
                return EVENT_PC_STRING;
            }
            break;

            case protocol::midi::MessageType::AfterTouchChannel:
            {
                return EVENT_CH_AT_STRING;
            }
            break;

            case protocol::midi::MessageType::AfterTouchPoly:
            {
                return EVENT_AT_STRING;
            }
            break;

            case protocol::midi::MessageType::PitchBend:
            {
                return EVENT_PB_STRING;
            }
            break;

            case protocol::midi::MessageType::SysEx:
            {
                return EVENT_SYS_EX_CONFIG_STRING;
            }
            break;

            case protocol::midi::MessageType::SysRealTimeClock:
            {
                return EVENT_RT_CLOCK_STRING;
            }
            break;

            case protocol::midi::MessageType::SysRealTimeStart:
            {
                return EVENT_RT_START_STRING;
            }
            break;

            case protocol::midi::MessageType::SysRealTimeContinue:
            {
                return EVENT_RT_CONTINUE_STRING;
            }
            break;

            case protocol::midi::MessageType::SysRealTimeStop:
            {
                return EVENT_RT_STOP_STRING;
            }
            break;

            case protocol::midi::MessageType::SysRealTimeActiveSensing:
            {
                return EVENT_RT_ASENS_STRING;
            }
            break;

            case protocol::midi::MessageType::SysRealTimeSystemReset:
            {
                return EVENT_RT_SYSRST_STRING;
            }
            break;

            case protocol::midi::MessageType::MmcPlay:
            {
                return EVENT_MMC_PLAY_STRING;
            }
            break;

            case protocol::midi::MessageType::MmcStop:
            {
                return EVENT_MMC_STOP_STRING;
            }
            break;

            case protocol::midi::MessageType::MmcPause:
            {
                return EVENT_MMC_PAUSE_STRING;
            }
            break;

            case protocol::midi::MessageType::MmcRecordStart:
            {
                return EVENT_MMC_RECORD_ON_STRING;
            }
            break;

            case protocol::midi::MessageType::MmcRecordStop:
            {
                return EVENT_MMC_RECORD_OFF_STRING;
            }
            break;

            case protocol::midi::MessageType::Nrpn7Bit:
            case protocol::midi::MessageType::Nrpn14Bit:
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

        /**
         * @brief Returns the display label for one note tonic.
         *
         * @param note Note tonic to stringify.
         *
         * @return Display string matching the note tonic.
         */
        static constexpr const char* note(protocol::midi::Note note)
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
