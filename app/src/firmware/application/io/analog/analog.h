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
#include "application/messaging/messaging.h"
#include "application/system/config.h"
#include "application/io/base.h"
#include "application/protocol/midi/midi.h"

#include <optional>

namespace io::analog
{
    class Analog : public io::Base
    {
        public:
        Analog(Hwa&      hwa,
               Filter&   filter,
               Database& database);

        bool   init() override;
        void   updateSingle(size_t index, bool forceRefresh = false) override;
        void   updateAll(bool forceRefresh = false) override;
        size_t maxComponentUpdateIndex() override;
        void   reset(size_t index);

        private:
        struct Descriptor
        {
            type_t           type        = type_t::POTENTIOMETER_CONTROL_CHANGE;
            bool             inverted    = false;
            uint16_t         lowerLimit  = 0;
            uint16_t         upperLimit  = 0;
            uint8_t          lowerOffset = 0;
            uint8_t          upperOffset = 0;
            uint16_t         maxValue    = 127;
            uint16_t         newValue    = 0;
            uint16_t         oldValue    = 0;
            messaging::Event event       = {};
        };

        static constexpr protocol::midi::messageType_t INTERNAL_MSG_TO_MIDI_TYPE[static_cast<uint8_t>(type_t::AMOUNT)] = {
            protocol::midi::messageType_t::CONTROL_CHANGE,          // POTENTIOMETER_CONTROL_CHANGE
            protocol::midi::messageType_t::NOTE_ON,                 // POTENTIOMETER_NOTE
            protocol::midi::messageType_t::NOTE_ON,                 // FSR (set to note off when appropriate)
            protocol::midi::messageType_t::INVALID,                 // BUTTON (let other listeners handle this)
            protocol::midi::messageType_t::NRPN_7BIT,               // NRPN_7BIT
            protocol::midi::messageType_t::NRPN_14BIT,              // NRPN_14BIT
            protocol::midi::messageType_t::PITCH_BEND,              // PITCH_BEND
            protocol::midi::messageType_t::CONTROL_CHANGE_14BIT,    // CONTROL_CHANGE_14BIT
            protocol::midi::messageType_t::INVALID,                 // RESERVED
        };

        Hwa&      _hwa;
        Filter&   _filter;
        Database& _database;
        uint8_t   _fsrPressed[Collection::SIZE() / 8 + 1] = {};
        uint16_t  _lastValue[Collection::SIZE()]          = {};

        void                   fillDescriptor(size_t index, Descriptor& descriptor);
        void                   processReading(size_t index, uint16_t value);
        bool                   checkPotentiometerValue(size_t index, Descriptor& descriptor);
        bool                   checkFSRvalue(size_t index, Descriptor& descriptor);
        void                   sendMessage(size_t index, Descriptor& descriptor);
        void                   setFSRstate(size_t index, bool state);
        bool                   fsrState(size_t index);
        std::optional<uint8_t> sysConfigGet(sys::Config::Section::analog_t section, size_t index, uint16_t& value);
        std::optional<uint8_t> sysConfigSet(sys::Config::Section::analog_t section, size_t index, uint16_t value);
    };
}    // namespace io::analog