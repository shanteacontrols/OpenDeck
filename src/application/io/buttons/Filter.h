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
#include "core/src/general//Helpers.h"

namespace IO
{
    class ButtonsFilter : public IO::Buttons::Filter
    {
        public:
        ButtonsFilter()
        {
            for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
                buttonDebounceCounter[i] = buttonDebounceCompare;
        }

        bool isFiltered(size_t index, bool state, bool& filteredState) override
        {
            if (index >= MAX_NUMBER_OF_BUTTONS)
            {
                //don't debounce analog inputs and touchscreen buttons
                filteredState = state;
                return true;
            }

            auto debounceRelease = [&]() {
                buttonDebounceCounter[index] = (buttonDebounceCounter[index] << (uint16_t)1) | (uint16_t)state | buttonDebounceCompare;

                return (buttonDebounceCounter[index] == buttonDebounceCompare);
            };

            if (state)
            {
                //debounce only release
                buttonDebounceCounter[index] = 0xFFFF;
                filteredState                = true;
            }
            else
            {
                if (debounceRelease())
                {
                    filteredState = false;
                }
                else
                {
                    filteredState = true;
                }
            }

            return true;
        }

        void reset(size_t index) override
        {
            buttonDebounceCounter[index] = buttonDebounceCompare;
        }

        private:
        ///
        /// \brief Constant used to debounce button readings.
        /// Once new value has been read, shift old value to the left, append new value and
        /// append buttonDebounceCompare with OR operator. If final value is equal to this
        /// constant button release is stable.
        /// OpenDeck boards are refreshing button states every 500us.
        /// Configure debouncer for 5ms debounce time on release only.
        ///
        const uint16_t buttonDebounceCompare = 0xFC00;

        ///
        /// \brief Array holding debounce count for all buttons to avoid incorrect state detection.
        ///
        uint16_t buttonDebounceCounter[MAX_NUMBER_OF_BUTTONS] = {};
    };
}    // namespace IO