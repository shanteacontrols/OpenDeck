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

namespace io::leds
{
    class HwaHw : public Hwa
    {
        public:
        HwaHw() = default;

        void setState(size_t index, brightness_t brightness) override
        {
            board::io::digital_out::writeLedState(index, static_cast<board::io::digital_out::ledBrightness_t>(brightness));
        }

        size_t rgbFromOutput(size_t index) override
        {
            return board::io::digital_out::rgbFromOutput(index);
        }

        size_t rgbComponentFromRgb(size_t index, rgbComponent_t component) override
        {
            return board::io::digital_out::rgbComponentFromRgb(index,
                                                               static_cast<board::io::digital_out::rgbComponent_t>(component));
        }
    };
}    // namespace io::leds