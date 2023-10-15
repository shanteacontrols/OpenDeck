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

#pragma once

#include "application/io/common/Common.h"
#include "MIDI/transport/USB.h"
#include "MIDI/transport/Serial.h"
#include "MIDI/transport/BLE.h"
#include "application/protocol/ProtocolBase.h"
#include "application/database/Database.h"
#include "application/system/Config.h"
#include "application/protocol/ProtocolBase.h"
#include "application/messaging/Messaging.h"

namespace protocol
{
    class MIDI : public protocol::Base
    {
        public:
        // alias some types and functions from base midi class for easier access
        using messageType_t                           = MIDIlib::Base::messageType_t;
        using usbMIDIPacket_t                         = MIDIlib::USBMIDI::usbMIDIPacket_t;
        using bleMIDIPacket_t                         = MIDIlib::BLEMIDI::bleMIDIPacket_t;
        using noteOffType_t                           = MIDIlib::Base::noteOffType_t;
        using note_t                                  = MIDIlib::Base::note_t;
        using message_t                               = MIDIlib::Base::message_t;
        static constexpr auto&   MIDI_7BIT_VALUE_MAX  = MIDIlib::Base::MIDI_7BIT_VALUE_MAX;
        static constexpr auto&   MIDI_14BIT_VALUE_MAX = MIDIlib::Base::MIDI_14BIT_VALUE_MAX;
        static constexpr auto&   NOTE_TO_TONIC        = MIDIlib::Base::noteToTonic;
        static constexpr auto&   NOTE_TO_OCTAVE       = MIDIlib::Base::noteToOctave;
        static constexpr uint8_t USB_EVENT            = MIDIlib::USBMIDI::USB_EVENT;
        static constexpr uint8_t USB_DATA1            = MIDIlib::USBMIDI::USB_DATA1;
        static constexpr uint8_t USB_DATA2            = MIDIlib::USBMIDI::USB_DATA2;
        static constexpr uint8_t USB_DATA3            = MIDIlib::USBMIDI::USB_DATA3;

        static constexpr uint8_t MIDI_CHANNEL_OMNI = 17;

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

        class HWAUSB : public MIDIlib::USBMIDI::HWA
        {
            public:
            virtual ~HWAUSB() = default;

            virtual bool supported() = 0;
        };

        class HWADIN : public io::common::Allocatable, public MIDIlib::SerialMIDI::HWA
        {
            public:
            virtual ~HWADIN() = default;

            virtual bool supported()             = 0;
            virtual bool setLoopback(bool state) = 0;
        };

        class HWABLE : public MIDIlib::BLEMIDI::HWA
        {
            public:
            virtual ~HWABLE() = default;

            virtual bool supported() = 0;
        };

        using Database = database::User<database::Config::Section::global_t>;

        MIDI(HWAUSB&   hwaUSB,
             HWADIN&   hwaDIN,
             HWABLE&   hwaBLE,
             Database& database);

        bool init() override;
        bool deInit() override;
        void read() override;

        private:
        enum interface_t
        {
            INTERFACE_USB,
            INTERFACE_DIN,
            INTERFACE_BLE,
            INTERFACE_AMOUNT
        };

        bool                   isSettingEnabled(setting_t feature);
        bool                   isDinLoopbackRequired();
        std::optional<uint8_t> sysConfigGet(sys::Config::Section::global_t section, size_t index, uint16_t& value);
        std::optional<uint8_t> sysConfigSet(sys::Config::Section::global_t section, size_t index, uint16_t value);
        void                   sendMIDI(messaging::eventType_t source, const messaging::event_t& event);
        void                   setNoteOffMode(noteOffType_t type);
        bool                   setupUSBMIDI();
        bool                   setupDINMIDI();
        bool                   setupBLEMIDI();
        bool                   setupThru();

        HWAUSB&                                      _hwaUSB;
        HWADIN&                                      _hwaDIN;
        HWABLE&                                      _hwaBLE;
        MIDIlib::USBMIDI                             _usbMIDI = MIDIlib::USBMIDI(_hwaUSB);
        MIDIlib::SerialMIDI                          _dinMIDI = MIDIlib::SerialMIDI(_hwaDIN);
        MIDIlib::BLEMIDI                             _bleMIDI = MIDIlib::BLEMIDI(_hwaBLE);
        Database&                                    _database;
        std::array<MIDIlib::Base*, INTERFACE_AMOUNT> _midiInterface;
        bool                                         _clockTimerAllocated = false;
        size_t                                       _clockTimerIndex     = 0;
    };
}    // namespace protocol