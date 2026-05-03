/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "../driver_base.h"
#include "count.h"

#include <array>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define OPENDECK_LED_NATIVE_GPIO_ENTRY(index, node_id) GPIO_DT_SPEC_GET_BY_IDX(node_id, native_gpios, index)

namespace opendeck::io::leds
{
    /**
     * @brief LED driver that drives native GPIO outputs directly.
     */
    class Driver : public DriverBase
    {
        public:
        /**
         * @brief Configures all native GPIO outputs at construction time.
         */
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

        /**
         * @brief Flushes no additional state because writes are applied immediately.
         */
        void update() override
        {
        }

        /**
         * @brief Sets one GPIO-backed LED output.
         *
         * @param index Output index to update.
         * @param brightness Brightness value to apply.
         */
        void set_state(size_t index, Brightness brightness) override
        {
            if (index >= _gpios.size())
            {
                return;
            }

            gpio_pin_set_dt(&_gpios[index], brightness != Brightness::Off);
        }

        /**
         * @brief Maps a physical output index to the corresponding RGB LED index.
         *
         * @param index Output index to map.
         *
         * @return RGB LED index corresponding to the output.
         */
        size_t rgb_from_output(size_t index) override
        {
            auto rgb_index = index / 3;
            return rgb_index < rgb_led_count() ? rgb_index : (rgb_led_count() ? rgb_led_count() - 1 : 0);
        }

        /**
         * @brief Maps an RGB LED index and component to a physical output index.
         *
         * @param index RGB LED index to map.
         * @param component RGB component to map.
         *
         * @return Physical output index corresponding to the RGB component.
         */
        size_t rgb_component_from_rgb(size_t index, RgbComponent component) override
        {
            return index * 3 + static_cast<uint8_t>(component);
        }

        private:
        static constexpr size_t LED_OUTPUT_COUNT = OPENDECK_LED_OUTPUT_PHYSICAL_COUNT;

        std::array<gpio_dt_spec, LED_OUTPUT_COUNT> _gpios = { { LISTIFY(OPENDECK_LED_OUTPUT_PHYSICAL_COUNT, OPENDECK_LED_NATIVE_GPIO_ENTRY, (, ), DT_NODELABEL(opendeck_leds)) } };

        /**
         * @brief Returns the number of complete RGB LEDs exposed by the driver.
         *
         * @return Number of RGB LEDs.
         */
        static constexpr size_t rgb_led_count()
        {
            return LED_OUTPUT_COUNT / 3;
        }
    };
}    // namespace opendeck::io::leds

#undef OPENDECK_LED_NATIVE_GPIO_ENTRY
