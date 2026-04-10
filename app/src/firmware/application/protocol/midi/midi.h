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

#include "deps.h"
#include "application/io/common/common.h"
#include "application/protocol/base.h"
#include "application/database/database.h"
#include "application/system/config.h"
#include "application/protocol/base.h"
#include "application/messaging/messaging.h"
#include "zlibs/utils/misc/timer.h"

#include <optional>

namespace protocol::midi
{
    class Midi : public protocol::Base
    {
        public:
        Midi(HwaUsb&    hwaUSB,
             HwaSerial& hwaSerial,
             HwaBle&    hwaBLE,
             Database&  database);

        bool init() override;
        bool deInit() override;
        void read() override;

        private:
        enum interface_t
        {
            INTERFACE_USB,
            INTERFACE_SERIAL,
            INTERFACE_BLE,
            INTERFACE_AMOUNT
        };

        HwaUsb&                                        _hwaUsb;
        HwaSerial&                                     _hwaSerial;
        HwaBle&                                        _hwaBle;
        lib::midi::usb::Usb                            _usb    = lib::midi::usb::Usb(_hwaUsb);
        lib::midi::serial::Serial                      _serial = lib::midi::serial::Serial(_hwaSerial);
        lib::midi::ble::Ble                            _ble    = lib::midi::ble::Ble(_hwaBle);
        Database&                                      _database;
        std::array<lib::midi::Base*, INTERFACE_AMOUNT> _midiInterface;
        zlibs::utils::misc::Timer                      _clockTimer;
        bool                                           _standardNoteOff = true;

        bool                     isSettingEnabled(setting_t feature);
        bool                     isDinLoopbackRequired();
        std::optional<uint8_t>   sysConfigGet(sys::Config::Section::global_t section, size_t index, uint16_t& value);
        std::optional<uint8_t>   sysConfigSet(sys::Config::Section::global_t section, size_t index, uint16_t value);
        void                     send(const messaging::MidiSignal& event);
        void                     send(const messaging::UmpSignal& event);
        void                     setNoteOffMode(noteOffType_t type);
        bool                     setupUsb();
        bool                     setupSerial();
        bool                     setupBle();
        bool                     setupThru();
        messaging::MidiTransport interfaceToTransport(size_t index) const;
    };
}    // namespace protocol::midi
