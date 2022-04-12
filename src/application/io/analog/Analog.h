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

#include "database/Database.h"
#include "messaging/Messaging.h"
#include "io/common/Common.h"
#include "system/Config.h"
#include "io/IOBase.h"
#include "protocol/midi/MIDI.h"

#ifdef ADC_SUPPORTED

using namespace Protocol;

namespace IO
{
    class Analog : public IO::Base
    {
        public:
        class Collection : public Common::BaseCollection<NR_OF_ANALOG_INPUTS>
        {
            public:
            Collection() = delete;
        };

        enum
        {
            GROUP_ANALOG_INPUTS,
            GROUP_TOUCHSCREEN_COMPONENTS
        };

        enum class type_t : uint8_t
        {
            POTENTIOMETER_CONTROL_CHANGE,
            POTENTIOMETER_NOTE,
            FSR,
            BUTTON,
            NRPN_7BIT,
            NRPN_14BIT,
            PITCH_BEND,
            CONTROL_CHANGE_14BIT,
            DMX,
            AMOUNT
        };

        enum class pressureType_t : uint8_t
        {
            VELOCITY,
            AFTERTOUCH
        };

        class HWA
        {
            public:
            // should return true if the value has been refreshed, false otherwise
            virtual bool value(size_t index, uint16_t& value) = 0;
        };

        class Filter
        {
            public:
            struct descriptor_t
            {
                Analog::type_t type        = Analog::type_t::POTENTIOMETER_CONTROL_CHANGE;
                uint16_t       value       = 0;
                uint16_t       lowerOffset = 0;
                uint16_t       upperOffset = 0;
                uint16_t       maxValue    = 127;
            };

            virtual bool     isFiltered(size_t index, descriptor_t& descriptor) = 0;
            virtual uint16_t lastValue(size_t index)                            = 0;
            virtual void     reset(size_t index)                                = 0;
        };

        Analog(HWA&                hwa,
               Filter&             filter,
               Database::Instance& database);

        bool   init() override;
        void   updateSingle(size_t index, bool forceRefresh = false) override;
        void   updateAll(bool forceRefresh = false) override;
        size_t maxComponentUpdateIndex() override;
        void   reset(size_t index);

        private:
        struct analogDescriptor_t
        {
            type_t             type        = type_t::POTENTIOMETER_CONTROL_CHANGE;
            bool               inverted    = false;
            uint16_t           lowerLimit  = 0;
            uint16_t           upperLimit  = 0;
            uint8_t            lowerOffset = 0;
            uint8_t            upperOffset = 0;
            uint16_t           maxValue    = 127;
            Messaging::event_t event;

            analogDescriptor_t() = default;
        };

        void                   fillAnalogDescriptor(size_t index, analogDescriptor_t& descriptor);
        void                   processReading(size_t index, uint16_t value);
        bool                   checkPotentiometerValue(size_t index, analogDescriptor_t& descriptor);
        bool                   checkFSRvalue(size_t index, analogDescriptor_t& descriptor);
        void                   sendMessage(size_t index, analogDescriptor_t& descriptor);
        void                   setFSRstate(size_t index, bool state);
        bool                   fsrState(size_t index);
        std::optional<uint8_t> sysConfigGet(System::Config::Section::analog_t section, size_t index, uint16_t& value);
        std::optional<uint8_t> sysConfigSet(System::Config::Section::analog_t section, size_t index, uint16_t value);

        HWA&                _hwa;
        Filter&             _filter;
        Database::Instance& _database;

        uint8_t _fsrPressed[Collection::size() / 8 + 1] = {};

        static constexpr MIDI::messageType_t INTERNAL_MSG_TO_MIDI_TYPE[static_cast<uint8_t>(type_t::AMOUNT)] = {
            MIDI::messageType_t::CONTROL_CHANGE,
            MIDI::messageType_t::NOTE_ON,
            MIDI::messageType_t::NOTE_ON,    // fsr: set to off when appropriate
            MIDI::messageType_t::INVALID,    // button: let other listeners handle this
            MIDI::messageType_t::NRPN_7BIT,
            MIDI::messageType_t::NRPN_14BIT,
            MIDI::messageType_t::PITCH_BEND,
            MIDI::messageType_t::CONTROL_CHANGE_14BIT,
        };
    };
}    // namespace IO

#else
#include "stub/Analog.h"
#endif