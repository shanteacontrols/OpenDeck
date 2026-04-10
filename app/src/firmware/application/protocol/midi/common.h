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

#include <array>
#include <cstddef>
#include <cstdint>

#include "zlibs/utils/midi/transport/usb/transport_usb.h"
#include "zlibs/utils/midi/transport/serial/transport_serial.h"
#include "zlibs/utils/midi/transport/ble/transport_ble.h"

namespace lib
{
    namespace midi = zlibs::utils::midi;
}

namespace protocol::midi
{
    using Usb       = lib::midi::usb::Usb;
    using Ble       = lib::midi::ble::Ble;
    using Serial    = lib::midi::serial::Serial;
    using BlePacket = lib::midi::ble::Packet;
    using note_t    = lib::midi::Note;

    struct UsbPacket
    {
        std::array<uint8_t, 4> data = {};
    };

    struct SerialPacket
    {
        uint8_t data = 0;
    };

    enum class noteOffType_t : uint8_t
    {
        STANDARD_NOTE_OFF,
        NOTE_ON_ZERO_VEL,
    };

    enum class messageType_t : uint8_t
    {
        INVALID                      = static_cast<uint8_t>(lib::midi::MessageType::Invalid),
        NOTE_OFF                     = static_cast<uint8_t>(lib::midi::MessageType::NoteOff),
        NOTE_ON                      = static_cast<uint8_t>(lib::midi::MessageType::NoteOn),
        CONTROL_CHANGE               = static_cast<uint8_t>(lib::midi::MessageType::ControlChange),
        PROGRAM_CHANGE               = static_cast<uint8_t>(lib::midi::MessageType::ProgramChange),
        AFTER_TOUCH_CHANNEL          = static_cast<uint8_t>(lib::midi::MessageType::AfterTouchChannel),
        AFTER_TOUCH_POLY             = static_cast<uint8_t>(lib::midi::MessageType::AfterTouchPoly),
        PITCH_BEND                   = static_cast<uint8_t>(lib::midi::MessageType::PitchBend),
        SYS_EX                       = static_cast<uint8_t>(lib::midi::MessageType::SysEx),
        SYS_REAL_TIME_CLOCK          = static_cast<uint8_t>(lib::midi::MessageType::SysRealTimeClock),
        SYS_REAL_TIME_START          = static_cast<uint8_t>(lib::midi::MessageType::SysRealTimeStart),
        SYS_REAL_TIME_CONTINUE       = static_cast<uint8_t>(lib::midi::MessageType::SysRealTimeContinue),
        SYS_REAL_TIME_STOP           = static_cast<uint8_t>(lib::midi::MessageType::SysRealTimeStop),
        SYS_REAL_TIME_ACTIVE_SENSING = static_cast<uint8_t>(lib::midi::MessageType::SysRealTimeActiveSensing),
        SYS_REAL_TIME_SYSTEM_RESET   = static_cast<uint8_t>(lib::midi::MessageType::SysRealTimeSystemReset),
        MMC_PLAY                     = static_cast<uint8_t>(lib::midi::MessageType::MmcPlay),
        MMC_STOP                     = static_cast<uint8_t>(lib::midi::MessageType::MmcStop),
        MMC_PAUSE                    = static_cast<uint8_t>(lib::midi::MessageType::MmcPause),
        MMC_RECORD_START             = static_cast<uint8_t>(lib::midi::MessageType::MmcRecordStart),
        MMC_RECORD_STOP              = static_cast<uint8_t>(lib::midi::MessageType::MmcRecordStop),
        NOTE                         = 0x10,
        CONTROL_CHANGE_RESET         = 0x11,
        REAL_TIME_CLOCK              = 0x12,
        REAL_TIME_START              = 0x13,
        REAL_TIME_CONTINUE           = 0x14,
        REAL_TIME_STOP               = 0x15,
        REAL_TIME_ACTIVE_SENSING     = 0x16,
        REAL_TIME_SYSTEM_RESET       = 0x17,
        MMC_RECORD                   = 0x18,
        NONE                         = 0x19,
        PRESET_CHANGE                = 0x1A,
        MULTI_VAL_INC_RESET_NOTE     = 0x1B,
        MULTI_VAL_INC_DEC_NOTE       = 0x1C,
        MULTI_VAL_INC_RESET_CC       = 0x1D,
        MULTI_VAL_INC_DEC_CC         = 0x1E,
        NOTE_OFF_ONLY                = 0x1F,
        CONTROL_CHANGE0_ONLY         = 0x20,
        PROGRAM_CHANGE_INC           = 0x21,
        PROGRAM_CHANGE_DEC           = 0x22,
        PROGRAM_CHANGE_OFFSET_INC    = 0x23,
        PROGRAM_CHANGE_OFFSET_DEC    = 0x24,
        BPM_INC                      = 0x25,
        BPM_DEC                      = 0x26,
        MMC_PLAY_STOP                = 0x27,
        NRPN_7BIT                    = 0x28,
        NRPN_14BIT                   = 0x29,
        CONTROL_CHANGE_14BIT         = 0x2A,
        AMOUNT                       = 0x2B,
    };

    struct message_t
    {
        messageType_t type    = messageType_t::INVALID;
        uint8_t       channel = 0;
        uint16_t      data1   = 0;
        uint16_t      data2   = 0;
    };

    constexpr inline auto    MAX_VALUE_7BIT  = lib::midi::MAX_VALUE_7BIT;
    constexpr inline auto    MAX_VALUE_14BIT = lib::midi::MAX_VALUE_14BIT;
    constexpr inline uint8_t USB_EVENT       = 0;
    constexpr inline uint8_t USB_DATA1       = 1;
    constexpr inline uint8_t USB_DATA2       = 2;
    constexpr inline uint8_t USB_DATA3       = 3;
    constexpr inline uint8_t OMNI_CHANNEL    = 17;

    constexpr inline note_t NOTE_TO_TONIC(int8_t note)
    {
        return lib::midi::note_to_tonic(note);
    }

    constexpr inline uint8_t NOTE_TO_OCTAVE(int8_t note)
    {
        return lib::midi::note_to_octave(note);
    }

    constexpr inline bool IS_CHANNEL_MESSAGE(messageType_t type)
    {
        switch (type)
        {
        case messageType_t::NOTE_OFF:
        case messageType_t::NOTE_ON:
        case messageType_t::CONTROL_CHANGE:
        case messageType_t::PROGRAM_CHANGE:
        case messageType_t::AFTER_TOUCH_CHANNEL:
        case messageType_t::AFTER_TOUCH_POLY:
        case messageType_t::PITCH_BEND:
        case messageType_t::NRPN_7BIT:
        case messageType_t::NRPN_14BIT:
        case messageType_t::CONTROL_CHANGE_14BIT:
            return true;

        default:
            return false;
        }
    }

    inline messageType_t decode_type(const midi_ump& packet)
    {
        return static_cast<messageType_t>(static_cast<uint8_t>(lib::midi::type(packet)));
    }

    inline message_t decode_message(const midi_ump& packet)
    {
        message_t message = {};
        message.type      = decode_type(packet);

        if (!lib::midi::is_channel_message(lib::midi::type(packet)))
        {
            return message;
        }

        message.channel = UMP_MIDI_CHANNEL(packet) + 1;
        message.data1   = UMP_MIDI1_P1(packet);
        message.data2   = UMP_MIDI1_P2(packet);

        if (message.type == messageType_t::PITCH_BEND)
        {
            message.data2 = lib::midi::Merge14Bit(UMP_MIDI1_P2(packet), UMP_MIDI1_P1(packet)).value();
        }
        else if (message.type == messageType_t::NOTE_OFF)
        {
            message.data2 = 0;
        }

        return message;
    }

    // convulted order to keep compatibility with older firmware
    enum class setting_t : uint8_t
    {
        STANDARD_NOTE_OFF,
        RUNNING_STATUS,
        DIN_THRU_USB,
        DIN_ENABLED,
        USB_THRU_DIN,
        USB_THRU_USB,
        USB_THRU_BLE,
        DIN_THRU_DIN,
        DIN_THRU_BLE,
        BLE_ENABLED,
        BLE_THRU_DIN,
        BLE_THRU_USB,
        BLE_THRU_BLE,
        USE_GLOBAL_CHANNEL,
        GLOBAL_CHANNEL,
        SEND_MIDI_CLOCK_DIN,
        AMOUNT
    };
}    // namespace protocol::midi
