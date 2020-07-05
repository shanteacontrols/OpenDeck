/*

Copyright 2015-2020 Igor Petrovic

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

#include "io/buttons/Buttons.h"

namespace IO
{
    class ButtonsFilter : public IO::Buttons::Filter
    {
        public:
        ButtonsFilter() {}

        bool isFiltered(size_t index, bool state, bool& filteredState) override
        {
            // Shift old value to the left, append new value and
            // append DEBOUNCE_COMPARE with OR command. If final value is equal to 0xFF or
            // DEBOUNCE_COMPARE, signal is debounced.
            buttonDebounceCounter[index] = (buttonDebounceCounter[index] << (uint8_t)1) | (uint8_t)state | buttonDebounceCompare;

            if ((buttonDebounceCounter[index] == buttonDebounceCompare) || (buttonDebounceCounter[index] == 0xFF))
            {
                filteredState = buttonDebounceCounter[index] == 0xFF;
                return true;
            }

            return false;
        }

        void reset(size_t index) override
        {
            buttonDebounceCounter[index] = 0;
        }

        private:
        ///
        /// \brief Constant used to debounce button readings.
        /// Once new value has been read, shift old value to the left, append new value and
        /// append buttonDebounceCompare with OR operator. If final value is equal to 0xFF or
        /// buttonDebounceCompare, button state is debounced.
        ///
        const uint8_t buttonDebounceCompare = 0b11110000;

        ///
        /// \brief Array holding debounce count for all buttons to avoid incorrect state detection.
        ///
        uint8_t buttonDebounceCounter[MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_TOUCHSCREEN_BUTTONS] = {};
    };
}    // namespace IO