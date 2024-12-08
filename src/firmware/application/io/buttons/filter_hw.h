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

namespace io::buttons
{
    class FilterHw : public Filter
    {
        public:
        FilterHw() = default;

        bool isFiltered(size_t index, uint8_t& numberOfReadings, uint16_t& states) override
        {
            /*
                This filter makes use of the fact that the difference between digital readings is 1ms.
                If the _debounceState is 0xFF or 0x00, the button is considered debounced. Since all bits
                are used in a byte variable, and each reading takes 1ms, the debounce time is 8ms.

                Technically, two readings are possible since maximum number of board readings is 16, and
                full debounce cycle takes 8 readings. Assume the boards aren't that slow that the difference
                between two calls of this function for the same button index is more than 8ms.
             */

            numberOfReadings = 1;

            if (index >= Collection::SIZE(GROUP_DIGITAL_INPUTS))
            {
                // don't debounce analog inputs and touchscreen buttons
                states = states & 0x01;
                return true;
            }

            _debounceState[index] <<= numberOfReadings;
            _debounceState[index] |= static_cast<uint8_t>(states);

            if (_debounceState[index] == 0xFF)
            {
                states = 1;
            }
            else if (_debounceState[index] == 0)
            {
                states = 0;
            }
            else
            {
                // not debounced yet
                return false;
            }

            return true;
        }

        private:
        uint8_t _debounceState[Collection::SIZE(GROUP_DIGITAL_INPUTS)] = {};
    };
}    // namespace io::buttons