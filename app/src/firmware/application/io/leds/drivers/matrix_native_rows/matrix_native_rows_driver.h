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

#include "../driver_base.h"
#include "count.h"

namespace io::leds
{
    class Driver : public DriverBase
    {
        public:
        void update() override
        {
        }

        void setState(size_t index, brightness_t brightness) override
        {
        }

        size_t rgbFromOutput(size_t index) override
        {
            const uint8_t columns  = DT_PROP(DT_CHOSEN(opendeck_leds), columns);
            const uint8_t row      = index / columns;
            const uint8_t mod      = row % 3;
            const uint8_t base_row = row - mod;
            const uint8_t column   = index % columns;
            const uint8_t result   = (base_row * columns) / 3 + column;
            return result < rgbLedCount() ? result : (rgbLedCount() ? rgbLedCount() - 1 : 0);
        }

        size_t rgbComponentFromRgb(size_t index, rgbComponent_t component) override
        {
            const uint8_t columns = DT_PROP(DT_CHOSEN(opendeck_leds), columns);
            const uint8_t column  = index % columns;
            const uint8_t row     = (index / columns) * 3;
            const uint8_t address = column + columns * row;
            return address + columns * static_cast<uint8_t>(component);
        }

        private:
        static constexpr size_t rgbLedCount()
        {
            return (DT_PROP(DT_CHOSEN(opendeck_leds), rows) * DT_PROP(DT_CHOSEN(opendeck_leds), columns)) / 3;
        }
    };
}    // namespace io::leds
