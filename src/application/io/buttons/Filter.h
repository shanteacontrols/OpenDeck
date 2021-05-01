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

#include "io/buttons/Buttons.h"
#include "core/src/general/Helpers.h"

namespace IO
{
    class ButtonsFilter : public IO::Buttons::Filter
    {
        public:
        ButtonsFilter()
        {
            for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
                _lastPressTime[i] = 1;
        }

        bool isFiltered(size_t index, bool state, bool& filteredState, uint32_t sampleTakenTime) override
        {
            if (index >= MAX_NUMBER_OF_BUTTONS)
            {
                //don't debounce analog inputs and touchscreen buttons
                filteredState = state;
                return true;
            }

            if (state)
            {
                //debounce only release
                filteredState         = true;
                _lastPressTime[index] = 0;
            }
            else
            {
                if (!_lastPressTime[index])
                    _lastPressTime[index] = sampleTakenTime;

                if ((sampleTakenTime - _lastPressTime[index]) > DEBOUNCE_RELEASE_TIME)
                    filteredState = false;
                else
                    filteredState = true;
            }

            return true;
        }

        void reset(size_t index) override
        {
            if (index >= MAX_NUMBER_OF_BUTTONS)
                return;

            _lastPressTime[index] = 1;
        }

        private:
        static constexpr uint32_t DEBOUNCE_RELEASE_TIME                 = 5;
        uint32_t                  _lastPressTime[MAX_NUMBER_OF_BUTTONS] = {};
    };
}    // namespace IO