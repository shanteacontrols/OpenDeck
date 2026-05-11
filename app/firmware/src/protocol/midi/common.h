/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "zlibs/utils/midi/transport/usb/transport_usb.h"
#include "zlibs/utils/midi/transport/serial/transport_serial.h"
#include "zlibs/utils/midi/transport/ble/transport_ble.h"

namespace opendeck::protocol::midi
{
    /**
     * @brief Maximum number of UMP packets accumulated before a USB burst is flushed.
     */
    static constexpr size_t USB_UMP_BURST_PACKET_COUNT = 64;

    /** @brief USB transport type alias used by the MIDI subsystem. */
    using Usb = zlibs::utils::midi::usb::Usb;
    /** @brief BLE transport type alias used by the MIDI subsystem. */
    using Ble = zlibs::utils::midi::ble::Ble;
    /** @brief Serial transport type alias used by the MIDI subsystem. */
    using Serial = zlibs::utils::midi::serial::Serial;
    /** @brief BLE packet type alias used by the MIDI subsystem. */
    using BlePacket = zlibs::utils::midi::ble::Packet;
    /** @brief Musical note type alias used by helper conversion functions. */
    using Note = zlibs::utils::midi::Note;

    /**
     * @brief Raw USB MIDI packet container.
     */
    struct UsbPacket
    {
        std::array<uint8_t, 4> data = {};
    };

    /**
     * @brief Raw serial MIDI packet container.
     */
    struct SerialPacket
    {
        uint8_t data = 0;
    };

    /**
     * @brief Selects how note-off events are encoded on output.
     */
    enum class NoteOffType : uint8_t
    {
        StandardNoteOff,
        NoteOnZeroVel,
    };

    /**
     * @brief Identifies the logical MIDI message type used by the application.
     */
    enum class MessageType : uint8_t
    {
        Invalid                  = static_cast<uint8_t>(zlibs::utils::midi::MessageType::Invalid),
        NoteOff                  = static_cast<uint8_t>(zlibs::utils::midi::MessageType::NoteOff),
        NoteOn                   = static_cast<uint8_t>(zlibs::utils::midi::MessageType::NoteOn),
        ControlChange            = static_cast<uint8_t>(zlibs::utils::midi::MessageType::ControlChange),
        ProgramChange            = static_cast<uint8_t>(zlibs::utils::midi::MessageType::ProgramChange),
        AfterTouchChannel        = static_cast<uint8_t>(zlibs::utils::midi::MessageType::AfterTouchChannel),
        AfterTouchPoly           = static_cast<uint8_t>(zlibs::utils::midi::MessageType::AfterTouchPoly),
        PitchBend                = static_cast<uint8_t>(zlibs::utils::midi::MessageType::PitchBend),
        SysEx                    = static_cast<uint8_t>(zlibs::utils::midi::MessageType::SysEx),
        SysRealTimeClock         = static_cast<uint8_t>(zlibs::utils::midi::MessageType::SysRealTimeClock),
        SysRealTimeStart         = static_cast<uint8_t>(zlibs::utils::midi::MessageType::SysRealTimeStart),
        SysRealTimeContinue      = static_cast<uint8_t>(zlibs::utils::midi::MessageType::SysRealTimeContinue),
        SysRealTimeStop          = static_cast<uint8_t>(zlibs::utils::midi::MessageType::SysRealTimeStop),
        SysRealTimeActiveSensing = static_cast<uint8_t>(zlibs::utils::midi::MessageType::SysRealTimeActiveSensing),
        SysRealTimeSystemReset   = static_cast<uint8_t>(zlibs::utils::midi::MessageType::SysRealTimeSystemReset),
        MmcPlay                  = static_cast<uint8_t>(zlibs::utils::midi::MessageType::MmcPlay),
        MmcStop                  = static_cast<uint8_t>(zlibs::utils::midi::MessageType::MmcStop),
        MmcPause                 = static_cast<uint8_t>(zlibs::utils::midi::MessageType::MmcPause),
        MmcRecordStart           = static_cast<uint8_t>(zlibs::utils::midi::MessageType::MmcRecordStart),
        MmcRecordStop            = static_cast<uint8_t>(zlibs::utils::midi::MessageType::MmcRecordStop),
        Note                     = 0x10,
        ControlChangeReset       = 0x11,
        RealTimeClock            = 0x12,
        RealTimeStart            = 0x13,
        RealTimeContinue         = 0x14,
        RealTimeStop             = 0x15,
        RealTimeActiveSensing    = 0x16,
        RealTimeSystemReset      = 0x17,
        MmcRecord                = 0x18,
        None                     = 0x19,
        PresetChange             = 0x1A,
        MultiValIncResetNote     = 0x1B,
        MultiValIncDecNote       = 0x1C,
        MultiValIncResetCc       = 0x1D,
        MultiValIncDecCc         = 0x1E,
        NoteOffOnly              = 0x1F,
        ControlChange0Only       = 0x20,
        ProgramChangeInc         = 0x21,
        ProgramChangeDec         = 0x22,
        ProgramChangeOffsetInc   = 0x23,
        ProgramChangeOffsetDec   = 0x24,
        BpmInc                   = 0x25,
        BpmDec                   = 0x26,
        MmcPlayStop              = 0x27,
        Nrpn7Bit                 = 0x28,
        Nrpn14Bit                = 0x29,
        ControlChange14Bit       = 0x2A,
        Count                    = 0x2B,
    };

    /**
     * @brief Decoded MIDI message used across the application.
     */
    struct Message
    {
        MessageType type    = MessageType::Invalid;
        uint8_t     channel = 0;
        uint16_t    data1   = 0;
        uint16_t    data2   = 0;
    };

    /** @brief Maximum 7-bit MIDI value. */
    constexpr inline auto MAX_VALUE_7BIT = zlibs::utils::midi::MAX_VALUE_7BIT;
    /** @brief Maximum 14-bit MIDI value. */
    constexpr inline auto MAX_VALUE_14BIT = zlibs::utils::midi::MAX_VALUE_14BIT;
    /** @brief Highest controller index eligible for paired 14-bit CC messages. */
    constexpr inline uint8_t CONTROL_CHANGE_14BIT_MAX_INDEX = 96;
    /** @brief Center value used by MIDI pitch bend. */
    constexpr inline uint16_t MIDI_PITCH_BEND_CENTER = 8192;
    /** @brief USB packet byte index holding the event field. */
    constexpr inline uint8_t USB_EVENT = 0;
    /** @brief USB packet byte index holding the first data byte. */
    constexpr inline uint8_t USB_DATA1 = 1;
    /** @brief USB packet byte index holding the second data byte. */
    constexpr inline uint8_t USB_DATA2 = 2;
    /** @brief USB packet byte index holding the third data byte. */
    constexpr inline uint8_t USB_DATA3 = 3;
    /** @brief Pseudo-channel value used to represent omni mode. */
    constexpr inline uint8_t OMNI_CHANNEL = 17;

    /**
     * @brief Converts a MIDI note number to its tonic within an octave.
     *
     * @param note MIDI note number.
     *
     * @return Tonic corresponding to the note.
     */
    constexpr inline Note note_to_tonic(int8_t note)
    {
        return zlibs::utils::midi::note_to_tonic(note);
    }

    /**
     * @brief Converts a MIDI note number to its octave number.
     *
     * @param note MIDI note number.
     *
     * @return Octave number corresponding to the note.
     */
    constexpr inline uint8_t note_to_octave(int8_t note)
    {
        return zlibs::utils::midi::note_to_octave(note);
    }

    /**
     * @brief Returns whether the message type carries a MIDI channel field.
     *
     * @param type Message type to inspect.
     *
     * @return `true` for channel messages, otherwise `false`.
     */
    constexpr inline bool is_channel_message(MessageType type)
    {
        switch (type)
        {
        case MessageType::NoteOff:
        case MessageType::NoteOn:
        case MessageType::ControlChange:
        case MessageType::ProgramChange:
        case MessageType::AfterTouchChannel:
        case MessageType::AfterTouchPoly:
        case MessageType::PitchBend:
        case MessageType::Nrpn7Bit:
        case MessageType::Nrpn14Bit:
        case MessageType::ControlChange14Bit:
            return true;

        default:
            return false;
        }
    }

    /**
     * @brief Decodes the application message type from one UMP packet.
     *
     * @param packet UMP packet to decode.
     *
     * @return Decoded message type, or `MessageType::Invalid` when unsupported.
     */
    inline MessageType decode_type(const midi_ump& packet)
    {
        const auto decoded_type = static_cast<uint8_t>(zlibs::utils::midi::type(packet));

        switch (decoded_type)
        {
        case static_cast<uint8_t>(zlibs::utils::midi::MessageType::Invalid):
        case static_cast<uint8_t>(zlibs::utils::midi::MessageType::NoteOff):
        case static_cast<uint8_t>(zlibs::utils::midi::MessageType::NoteOn):
        case static_cast<uint8_t>(zlibs::utils::midi::MessageType::ControlChange):
        case static_cast<uint8_t>(zlibs::utils::midi::MessageType::ProgramChange):
        case static_cast<uint8_t>(zlibs::utils::midi::MessageType::AfterTouchChannel):
        case static_cast<uint8_t>(zlibs::utils::midi::MessageType::AfterTouchPoly):
        case static_cast<uint8_t>(zlibs::utils::midi::MessageType::PitchBend):
        case static_cast<uint8_t>(zlibs::utils::midi::MessageType::SysEx):
        case static_cast<uint8_t>(zlibs::utils::midi::MessageType::SysRealTimeClock):
        case static_cast<uint8_t>(zlibs::utils::midi::MessageType::SysRealTimeStart):
        case static_cast<uint8_t>(zlibs::utils::midi::MessageType::SysRealTimeContinue):
        case static_cast<uint8_t>(zlibs::utils::midi::MessageType::SysRealTimeStop):
        case static_cast<uint8_t>(zlibs::utils::midi::MessageType::SysRealTimeActiveSensing):
        case static_cast<uint8_t>(zlibs::utils::midi::MessageType::SysRealTimeSystemReset):
        case static_cast<uint8_t>(zlibs::utils::midi::MessageType::MmcPlay):
        case static_cast<uint8_t>(zlibs::utils::midi::MessageType::MmcStop):
        case static_cast<uint8_t>(zlibs::utils::midi::MessageType::MmcPause):
        case static_cast<uint8_t>(zlibs::utils::midi::MessageType::MmcRecordStart):
        case static_cast<uint8_t>(zlibs::utils::midi::MessageType::MmcRecordStop):
            return static_cast<MessageType>(decoded_type);

        default:
            return MessageType::Invalid;
        }
    }

    /**
     * @brief Decodes one UMP packet into the application message representation.
     *
     * @param packet UMP packet to decode.
     *
     * @return Decoded message structure.
     */
    inline Message decode_message(const midi_ump& packet)
    {
        Message message = {};
        message.type    = decode_type(packet);

        if (!zlibs::utils::midi::is_channel_message(zlibs::utils::midi::type(packet)))
        {
            return message;
        }

        message.channel = UMP_MIDI_CHANNEL(packet) + 1;
        message.data1   = UMP_MIDI1_P1(packet);
        message.data2   = UMP_MIDI1_P2(packet);

        if (message.type == MessageType::PitchBend)
        {
            message.data2 = zlibs::utils::midi::Merge14Bit(UMP_MIDI1_P2(packet), UMP_MIDI1_P1(packet)).value();
        }
        else if (message.type == MessageType::NoteOff)
        {
            message.data2 = 0;
        }

        return message;
    }

    /**
     * @brief Identifies global MIDI subsystem settings stored in configuration.
     *
     * @note The ordering is preserved for compatibility with older firmware.
     */
    enum class Setting : uint8_t
    {
        StandardNoteOff,
        RunningStatus,
        DinThruUsb,
        DinEnabled,
        UsbThruDin,
        UsbThruUsb,
        UsbThruBle,
        DinThruDin,
        DinThruBle,
        BleEnabled,
        BleThruDin,
        BleThruUsb,
        BleThruBle,
        UseGlobalChannel,
        GlobalChannel,
        SendMidiClockDin,
        Count
    };
}    // namespace opendeck::protocol::midi
