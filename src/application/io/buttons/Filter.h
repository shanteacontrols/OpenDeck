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

namespace IO
{
    class ButtonsFilter : public IO::Buttons::Filter
    {
        public:
        ButtonsFilter() = default;

        bool isFiltered(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
        {
            //this is a board-optimized debouncer
            //by the time processing of buttons takes place, more than 5ms has already passed
            //5ms is release debounce time
            //take into account only two latest readings
            //if any of those is 1 (pressed), consider the button pressed
            //otherwise, button is considered released

            numberOfReadings = 1;

            if (index >= MAX_NUMBER_OF_BUTTONS)
            {
                //don't debounce analog inputs and touchscreen buttons
                states = states & 0x01;
                return true;
            }

            states &= 0x03;

            if (numberOfReadings >= 2)
            {
                if (states)
                {
                    //button is pressed
                    states = 0x01;
                }
            }
            else
            {
                states &= 0x01;
            }

            return true;
        }
    };
}    // namespace IO