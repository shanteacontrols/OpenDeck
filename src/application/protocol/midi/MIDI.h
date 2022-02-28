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
#include "midi/src/MIDI.h"
#include "protocol/ProtocolBase.h"
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

        class HWAUSB
        {
            public:
            HWAUSB() = default;

            virtual bool supported()                                   = 0;
            virtual bool init()                                        = 0;
            virtual bool deInit()                                      = 0;
            virtual bool read(::MIDI::usbMIDIPacket_t& USBMIDIpacket)  = 0;
            virtual bool write(::MIDI::usbMIDIPacket_t& USBMIDIpacket) = 0;
        };

        class HWADIN : public IO::Common::Allocatable
        {
            public:
            HWADIN() = default;

            virtual bool supported()             = 0;
            virtual bool init()                  = 0;
            virtual bool deInit()                = 0;
            virtual bool read(uint8_t& data)     = 0;
            virtual bool write(uint8_t data)     = 0;
            virtual bool setLoopback(bool state) = 0;
        };

        MIDI(HWAUSB&   hwaUSB,
             HWADIN&   hwaDIN,
             Database& database);

        bool init() override;
        bool deInit() override;
        void read() override;

        private:
        class HWAInternal : public ::MIDI::HWA
        {
            public:
            HWAInternal(Protocol::MIDI& midi)
                : _midi(midi)
            {}

            bool init(::MIDI::interface_t interface) override;
            bool deInit(::MIDI::interface_t interface) override;
            bool dinRead(uint8_t& value) override;
            bool dinWrite(uint8_t value) override;
            bool usbRead(::MIDI::usbMIDIPacket_t& packet) override;
            bool usbWrite(::MIDI::usbMIDIPacket_t& packet) override;

            private:
            Protocol::MIDI& _midi;
            bool            _dinMIDIenabled         = false;
            bool            _dinMIDIloopbackEnabled = false;
        };

        bool                   isFeatureEnabled(feature_t feature);
        mergeType_t            mergeType();
        std::optional<uint8_t> sysConfigGet(System::Config::Section::global_t section, size_t index, uint16_t& value);
        std::optional<uint8_t> sysConfigSet(System::Config::Section::global_t section, size_t index, uint16_t value);

        HWAUSB&     _hwaUSB;
        HWADIN&     _hwaDIN;
        HWAInternal _hwaInternal;
        Database&   _database;
    };
}    // namespace Protocol