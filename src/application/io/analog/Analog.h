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

#include "database/Database.h"
#include "util/messaging/Messaging.h"
#include "io/common/Common.h"
#include "system/Config.h"
#include "io/IOBase.h"

#if defined(ADC_SUPPORTED) || defined(TOUCHSCREEN_SUPPORTED)
#define ANALOG_SUPPORTED

namespace IO
{
    class Analog : public IO::Base
    {
        public:
        class Collection : public Common::BaseCollection<NR_OF_ANALOG_INPUTS,
                                                         NR_OF_TOUCHSCREEN_COMPONENTS>
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
            potentiometerControlChange,
            potentiometerNote,
            fsr,
            button,
            nrpn7bit,
            nrpn14bit,
            pitchBend,
            controlChange14bit,
            AMOUNT
        };

        enum class pressureType_t : uint8_t
        {
            velocity,
            aftertouch
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
            enum class adcType_t : uint16_t
            {
                adc10bit = 1023,
                adc12bit = 4095
            };

            virtual bool isFiltered(size_t index, Analog::type_t type, uint16_t value, uint16_t& filteredValue) = 0;
            virtual void reset(size_t index)                                                                    = 0;
        };

        Analog(HWA&      hwa,
               Filter&   filter,
               Database& database);

        bool init() override;
        void update(bool forceRefresh = false) override;
        void reset(size_t index);

        private:
        struct analogDescriptor_t
        {
            type_t                             type       = type_t::potentiometerControlChange;
            bool                               inverted   = false;
            uint16_t                           lowerLimit = 0;
            uint16_t                           upperLimit = 0;
            Util::MessageDispatcher::message_t dispatchMessage;

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

        HWA&      _hwa;
        Filter&   _filter;
        Database& _database;

        uint8_t  _fsrPressed[Collection::size() / 8 + 1] = {};
        uint16_t _lastValue[Collection::size()]          = {};

        const MIDI::messageType_t _internalMsgToMIDIType[static_cast<uint8_t>(type_t::AMOUNT)] = {
            MIDI::messageType_t::controlChange,
            MIDI::messageType_t::noteOn,
            MIDI::messageType_t::noteOn,     // fsr: set to off when appropriate
            MIDI::messageType_t::invalid,    // button: let other listeners handle this
            MIDI::messageType_t::nrpn7bit,
            MIDI::messageType_t::nrpn14bit,
            MIDI::messageType_t::pitchBend,
            MIDI::messageType_t::controlChange14bit,
        };
    };
}    // namespace IO

#else
#include "stub/Analog.h"
#endif