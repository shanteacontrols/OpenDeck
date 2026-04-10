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

#include "zlibs/utils/misc/bit.h"

#include <array>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

namespace io::leds
{
    class Driver : public DriverBase
    {
        public:
        Driver()
        {
            if (device_is_ready(_data.port))
            {
                gpio_pin_configure_dt(&_data, GPIO_OUTPUT_INACTIVE);
            }

            if (device_is_ready(_clock.port))
            {
                gpio_pin_configure_dt(&_clock, GPIO_OUTPUT_INACTIVE);
            }

            if (device_is_ready(_latch.port))
            {
                gpio_pin_configure_dt(&_latch, GPIO_OUTPUT_INACTIVE);
            }
        }

        void update() override
        {
        }

        void setState(size_t index, brightness_t brightness) override
        {
            if (index >= register_count * 8)
            {
                return;
            }

            const size_t byte = index / 8;
            const size_t bit  = index % 8;

            zlibs::utils::misc::bit_write(_state[byte], bit, brightness != brightness_t::OFF);
            flush();
        }

        size_t rgbFromOutput(size_t index) override
        {
            const size_t result = index / 3;
            return result < rgbLedCount() ? result : (rgbLedCount() ? rgbLedCount() - 1 : 0);
        }

        size_t rgbComponentFromRgb(size_t index, rgbComponent_t component) override
        {
            return index * 3 + static_cast<uint8_t>(component);
        }

        private:
        static constexpr size_t             register_count = DT_PROP(DT_CHOSEN(opendeck_leds), shift_registers);
        gpio_dt_spec                        _data          = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_leds), data_gpios);
        gpio_dt_spec                        _clock         = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_leds), clock_gpios);
        gpio_dt_spec                        _latch         = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_leds), latch_gpios);
        std::array<uint8_t, register_count> _state         = {};

        void flush()
        {
            gpio_pin_set_dt(&_latch, 0);

            for (size_t reg = 0; reg < register_count; reg++)
            {
                const uint8_t value = _state[reg];

                for (int bit = 7; bit >= 0; bit--)
                {
                    gpio_pin_set_dt(&_data, zlibs::utils::misc::bit_read(value, static_cast<size_t>(bit)));
                    gpio_pin_set_dt(&_clock, 0);
                    gpio_pin_set_dt(&_clock, 1);
                }
            }

            gpio_pin_set_dt(&_latch, 1);
        }

        static constexpr size_t rgbLedCount()
        {
            return (register_count * 8) / 3;
        }
    };
}    // namespace io::leds
