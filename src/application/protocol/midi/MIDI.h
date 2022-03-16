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

#include "io/common/Common.h"
#include "midi/src/transport/usb/USB.h"
#include "midi/src/transport/serial/Serial.h"
#include "protocol/ProtocolBase.h"
#include "database/Database.h"
#include "system/Config.h"
#include "protocol/ProtocolBase.h"
#include "messaging/Messaging.h"

namespace Protocol
{
    class MIDI : public Protocol::Base
    {
        public:
        // alias some types and functions from base midi class for easier access
        using messageType_t                            = MIDIlib::Base::messageType_t;
        using usbMIDIPacket_t                          = MIDIlib::USBMIDI::usbMIDIPacket_t;
        using noteOffType_t                            = MIDIlib::Base::noteOffType_t;
        using note_t                                   = MIDIlib::Base::note_t;
        using message_t                                = MIDIlib::Base::message_t;
        static constexpr auto&   MIDI_7_BIT_VALUE_MAX  = MIDIlib::Base::MIDI_7_BIT_VALUE_MAX;
        static constexpr auto&   MIDI_14_BIT_VALUE_MAX = MIDIlib::Base::MIDI_14_BIT_VALUE_MAX;
        static constexpr auto&   noteToTonic           = MIDIlib::Base::noteToTonic;
        static constexpr auto&   noteToOctave          = MIDIlib::Base::noteToOctave;
        static constexpr uint8_t USB_EVENT             = MIDIlib::USBMIDI::USB_EVENT;
        static constexpr uint8_t USB_DATA1             = MIDIlib::USBMIDI::USB_DATA1;
        static constexpr uint8_t USB_DATA2             = MIDIlib::USBMIDI::USB_DATA2;
        static constexpr uint8_t USB_DATA3             = MIDIlib::USBMIDI::USB_DATA3;

        static constexpr uint8_t MIDI_CHANNEL_OMNI = 17;

        // convulted order to keep compatibility with older firmware
        enum class setting_t : uint8_t
        {
            standardNoteOff,
            runningStatus,
            dinThruUsb,
            dinEnabled,
            usbThruDin,
            usbThruUsb,
            usbThruBle,
            dinThruDin,
            dinThruBle,
            bleEnabled,
            bleThruDin,
            bleThruUsb,
            bleThruBle,
            useGlobalChannel,
            globalChannel,
            AMOUNT
        };

        class HWAUSB : public MIDIlib::USBMIDI::HWA
        {
            public:
            virtual bool supported() = 0;
        };

        class HWADIN : public IO::Common::Allocatable, public MIDIlib::SerialMIDI::HWA
        {
            public:
            virtual bool supported()             = 0;
            virtual bool setLoopback(bool state) = 0;
        };

        MIDI(HWAUSB&             hwaUSB,
             HWADIN&             hwaDIN,
             Database::Instance& database);

        bool init() override;
        bool deInit() override;
        void read() override;

        private:
        enum interface_t
        {
            INTERFACE_USB,
            INTERFACE_DIN,
            INTERFACE_AMOUNT
        };

        bool                   isSettingEnabled(setting_t feature);
        bool                   isDinLoopbackRequired();
        std::optional<uint8_t> sysConfigGet(System::Config::Section::global_t section, size_t index, uint16_t& value);
        std::optional<uint8_t> sysConfigSet(System::Config::Section::global_t section, size_t index, uint16_t value);
        void                   sendMIDI(Messaging::eventType_t source, const Messaging::event_t& event);
        void                   setNoteOffMode(noteOffType_t type);
        bool                   setupUSBMIDI();
        bool                   setupDINMIDI();
        bool                   setupThru();

        HWAUSB&                                      _hwaUSB;
        HWADIN&                                      _hwaDIN;
        MIDIlib::USBMIDI                             _usbMIDI = MIDIlib::USBMIDI(_hwaUSB);
        MIDIlib::SerialMIDI                          _dinMIDI = MIDIlib::SerialMIDI(_hwaDIN);
        Database::Instance&                          _database;
        std::array<MIDIlib::Base*, INTERFACE_AMOUNT> _midiInterface;
    };
}    // namespace Protocol