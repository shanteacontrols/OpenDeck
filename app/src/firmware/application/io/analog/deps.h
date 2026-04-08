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

#include "common.h"

#include "application/database/database.h"
#include "application/io/buttons/common.h"

namespace io::analog
{
    using Database = database::User<database::Config::Section::analog_t>;

    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        virtual bool    value(size_t index, uint16_t& value) = 0;
        virtual uint8_t adcBits()                            = 0;
    };

    class Filter
    {
        public:
        struct Descriptor
        {
            type_t   type        = type_t::POTENTIOMETER_CONTROL_CHANGE;
            uint16_t value       = 0;
            uint16_t lowerOffset = 0;
            uint16_t upperOffset = 0;
            uint16_t maxValue    = 127;
        };

        virtual ~Filter() = default;

        virtual bool isFiltered(size_t index, Descriptor& descriptor) = 0;
        virtual void reset(size_t index)                              = 0;
    };
}    // namespace io::analog