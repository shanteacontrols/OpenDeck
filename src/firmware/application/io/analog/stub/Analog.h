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

#include "io/IOBase.h"
namespace io
{
    class Analog : public io::Base
    {
        public:
        class Collection : public common::BaseCollection<0>
        {
            public:
            Collection() = delete;
        };

        enum
        {
            GROUP_ANALOG_INPUTS,
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
            RESERVED,
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
            virtual ~HWA() = default;

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

            virtual ~Filter() = default;

            virtual bool isFiltered(size_t index, descriptor_t& descriptor) = 0;
            virtual void reset(size_t index)                                = 0;
        };

        using buttonHandler_t = std::function<void(size_t index, bool state)>;
        using Database        = database::User<database::Config::Section::analog_t>;

        Analog(HWA&      hwa,
               Filter&   filter,
               Database& database)
        {}

        bool init() override
        {
            return false;
        }

        void updateSingle(size_t index, bool forceRefresh = false) override
        {
        }

        void updateAll(bool forceRefresh = false) override
        {
        }

        size_t maxComponentUpdateIndex() override
        {
            return 0;
        }
    };
}    // namespace io