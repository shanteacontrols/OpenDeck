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

#include "application/io/touchscreen/deps.h"

#include "core/util/ring_buffer.h"

namespace io::touchscreen
{
    class Viewtech : public Model
    {
        public:
        Viewtech(Hwa& hwa);

        bool      init() override;
        bool      deInit() override;
        bool      setScreen(size_t index) override;
        tsEvent_t update(Data& data) override;
        void      setIconState(Icon& icon, bool state) override;
        bool      setBrightness(brightness_t brightness) override;

        private:
        enum class response_t : uint32_t
        {
            BUTTON_STATE_CHANGE = 0x05820002
        };

        // there are 7 levels of brighness - scale them to available range (0-64)
        static constexpr uint8_t BRIGHTNESS_MAPPING[7] = {
            6,
            16,
            32,
            48,
            51,
            58,
            64
        };

        Hwa& _hwa;
    };
}    // namespace io::touchscreen
