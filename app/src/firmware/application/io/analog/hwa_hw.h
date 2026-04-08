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
#include "board/board.h"

namespace io::analog
{
    class HwaHw : public Hwa
    {
        public:
        HwaHw() = default;

        bool value(size_t index, uint16_t& value) override
        {
            return board::io::analog::value(index, value);
        }

        uint8_t adcBits() override
        {
            // only 10 and 12-bit ADC supported
            return CORE_MCU_ADC_MAX_VALUE == 1023 ? 10 : 12;
        }
    };
}    // namespace io::analog