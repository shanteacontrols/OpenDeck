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

#include <stdio.h>
#include <stdlib.h>
#include "io/analog/Analog.h"

namespace IO
{
    class AnalogFilter : public Analog::Filter
    {
        public:
        static constexpr adcType_t ADC_RESOLUTION = adcType_t::adc10bit;

        AnalogFilter()
        {}

        bool isFiltered(size_t index, Analog::type_t type, uint16_t value, uint16_t& filteredValue) override
        {
            return false;
        }

        uint16_t lastValue(size_t index) override
        {
            return 0;
        }

        void reset(size_t index) override
        {
        }
    };
}    // namespace IO