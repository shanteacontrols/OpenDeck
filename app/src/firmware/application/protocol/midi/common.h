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

#include "lib/midi/transport/usb/usb.h"
#include "lib/midi/transport/serial/serial.h"
#include "lib/midi/transport/ble/ble.h"

namespace protocol::midi
{
    // alias some types and functions from base midi class for easier access

    using messageType_t = lib::midi::messageType_t;
    using Usb           = lib::midi::usb::Usb;
    using Ble           = lib::midi::ble::Ble;
    using Serial        = lib::midi::serial::Serial;
    using UsbPacket     = lib::midi::usb::Packet;
    using BlePacket     = lib::midi::ble::Packet;
    using SerialPacket  = lib::midi::serial::Packet;
    using noteOffType_t = lib::midi::noteOffType_t;
    using note_t        = lib::midi::note_t;
    using message_t     = lib::midi::Message;

    constexpr inline auto    MAX_VALUE_7BIT     = lib::midi::MAX_VALUE_7BIT;
    constexpr inline auto    MAX_VALUE_14BIT    = lib::midi::MAX_VALUE_14BIT;
    constexpr inline auto&   NOTE_TO_TONIC      = lib::midi::NOTE_TO_TONIC;
    constexpr inline auto&   NOTE_TO_OCTAVE     = lib::midi::NOTE_TO_OCTAVE;
    constexpr inline auto&   IS_CHANNEL_MESSAGE = lib::midi::IS_CHANNEL_MESSAGE;
    constexpr inline uint8_t USB_EVENT          = lib::midi::usb::Packet::USB_EVENT;
    constexpr inline uint8_t USB_DATA1          = lib::midi::usb::Packet::USB_DATA1;
    constexpr inline uint8_t USB_DATA2          = lib::midi::usb::Packet::USB_DATA2;
    constexpr inline uint8_t USB_DATA3          = lib::midi::usb::Packet::USB_DATA3;
    constexpr inline uint8_t OMNI_CHANNEL       = 17;

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