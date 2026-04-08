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

        /*
            This filter makes use of the fact that the difference between digital readings is 1ms.
            If the _debounceState is 0xFF or 0x00, the button is considered debounced. Since all bits
            are used in a byte variable, and each reading takes 1ms, the debounce time is 8ms.
        */
        bool isFiltered(size_t index, bool& state) override
        {
            if (index >= sizeof(_debounceState))
            {
                return true;
            }

            _debounceState[index] <<= 1;
            _debounceState[index] |= static_cast<uint8_t>(state);

            if (_debounceState[index] == 0xFF)
            {
                state = 1;
            }
            else if (_debounceState[index] == 0)
            {
                state = 0;
            }
            else
            {
                // not debounced yet
                return false;
            }

            return true;
        }

        private:
        // don't debounce analog inputs and touchscreen buttons
        uint8_t _debounceState[Collection::SIZE(GROUP_DIGITAL_INPUTS)] = {};
    };
}    // namespace io::buttons