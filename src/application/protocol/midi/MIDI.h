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

#pragma once

#include "io/common/Common.h"
#include "midi/src/MIDI.h"
#include "protocol/ProtocolBase.h"
#include "util/messaging/Messaging.h"
#include "database/Database.h"
#include "system/Config.h"
#include "protocol/ProtocolBase.h"

namespace Protocol
{
    class MIDI : public ::MIDI, public Protocol::Base
    {
        public:
        enum class feature_t : uint8_t
        {
            standardNoteOff,
            runningStatus,
            mergeEnabled,
            dinEnabled,
            passToDIN,
            AMOUNT
        };

        enum class mergeSetting_t : uint8_t
        {
            mergeType,
            mergeUSBchannel,
            mergeDINchannel,
            AMOUNT
        };

        enum class mergeType_t : uint8_t
        {
            DINtoUSB,
            DINtoDIN,
            AMOUNT
        };

        // More functionality from HWA MIDI class is required than what's specified there.
        // A level of indirection is used to work-around this:
        // * Protocol::MIDI::HWA interface is expected to construct Protocol::MIDI class
        // * ::MIDI::HWA is implemented internally which uses extended API calls from this interface
        class HWA : public IO::Common::Allocatable, public ::MIDI::HWA
        {
            public:
            HWA() = default;

            virtual bool dinSupported()             = 0;
            virtual bool setDINLoopback(bool state) = 0;
        };

        MIDI(HWA&      hwa,
             Database& database);

        bool init() override;
        void read() override;

        private:
        class HWAInternal : public ::MIDI::HWA
        {
            public:
            HWAInternal(Protocol::MIDI& midi)
                : _midi(midi)
            {}

            bool init(MIDI::interface_t interface) override;
            bool deInit(MIDI::interface_t interface) override;
            bool dinRead(uint8_t& value) override;
            bool dinWrite(uint8_t value) override;
            bool usbRead(MIDI::USBMIDIpacket_t& USBMIDIpacket) override;
            bool usbWrite(MIDI::USBMIDIpacket_t& USBMIDIpacket) override;

            private:
            Protocol::MIDI& _midi;
            bool            _dinMIDIenabled         = false;
            bool            _dinMIDIloopbackEnabled = false;
        };

        void                   sendMIDI(const Util::MessageDispatcher::message_t& message);
        bool                   isFeatureEnabled(feature_t feature);
        mergeType_t            mergeType();
        std::optional<uint8_t> sysConfigGet(System::Config::Section::global_t section, size_t index, uint16_t& value);
        std::optional<uint8_t> sysConfigSet(System::Config::Section::global_t section, size_t index, uint16_t value);

        HWA&        _hwa;
        HWAInternal _hwaInternal;
        Database&   _database;
    };
}    // namespace Protocol