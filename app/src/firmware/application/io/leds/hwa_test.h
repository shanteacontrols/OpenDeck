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

#include <gmock/gmock.h>

namespace io::leds
{
    class HwaTest : public Hwa
    {
        public:
        HwaTest() = default;

        MOCK_METHOD2(setState, void(size_t index, brightness_t brightness));

        size_t rgbComponentFromRgb(size_t index, rgbComponent_t component) override
        {
            return index * 3 + static_cast<uint8_t>(component);
        }

        size_t rgbFromOutput(size_t index) override
        {
            return index / 3;
        }
    };
}    // namespace io::leds