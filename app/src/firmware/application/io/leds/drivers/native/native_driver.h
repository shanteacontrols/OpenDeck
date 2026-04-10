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

#include <array>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define OPENDECK_LED_NATIVE_GPIO_ENTRY(index, node_id) GPIO_DT_SPEC_GET_BY_IDX(node_id, native_gpios, index)

namespace io::leds
{
    class Driver : public DriverBase
    {
        public:
        Driver()
        {
            for (const auto& gpio : _gpios)
            {
                if (device_is_ready(gpio.port))
                {
                    gpio_pin_configure_dt(&gpio, GPIO_OUTPUT_INACTIVE);
                }
            }
        }

        void update() override
        {
        }

        void setState(size_t index, brightness_t brightness) override
        {
            if (index >= _gpios.size())
            {
                return;
            }

            gpio_pin_set_dt(&_gpios[index], brightness != brightness_t::OFF);
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
        static constexpr size_t              gpio_count = OPENDECK_LED_OUTPUT_COUNT;
        std::array<gpio_dt_spec, gpio_count> _gpios     = { { LISTIFY(OPENDECK_LED_OUTPUT_COUNT, OPENDECK_LED_NATIVE_GPIO_ENTRY, (, ), DT_CHOSEN(opendeck_leds)) } };

        static constexpr size_t rgbLedCount()
        {
            return gpio_count / 3;
        }
    };
}    // namespace io::leds

#undef OPENDECK_LED_NATIVE_GPIO_ENTRY
