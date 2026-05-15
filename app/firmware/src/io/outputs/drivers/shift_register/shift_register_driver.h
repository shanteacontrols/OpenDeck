/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/drivers/driver_base.h"
#include "firmware/src/io/outputs/drivers/shift_register/count.h"

#include "zlibs/utils/misc/bit.h"

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include <array>

namespace opendeck::io::outputs
{
    /**
     * @brief OUTPUT driver that shifts output state through one or more shift registers.
     */
    class Driver : public DriverBase
    {
        public:
        /**
         * @brief Configures shift-register GPIO lines at construction time.
         */
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

        /**
         * @brief Flushes no additional state because writes are pushed immediately.
         */
        void update() override
        {
        }

        /**
         * @brief Sets one shift-register-driven OUTPUT output and flushes the register state.
         *
         * @param index Output index to update.
         * @param brightness Brightness value to apply.
         */
        void set_state(size_t index, Brightness brightness) override
        {
            if (index >= register_count * 8)
            {
                return;
            }

            const size_t byte = index / 8;
            const size_t bit  = index % 8;

            zlibs::utils::misc::bit_write(_state[byte], bit, brightness != Brightness::Off);
            flush();
        }

        /**
         * @brief Maps a physical output index to the corresponding RGB OUTPUT index.
         *
         * @param index Output index to map.
         *
         * @return RGB OUTPUT index corresponding to the output.
         */
        size_t rgb_from_output(size_t index) override
        {
            const size_t result = index / 3;
            return result < rgb_output_count() ? result : (rgb_output_count() ? rgb_output_count() - 1 : 0);
        }

        /**
         * @brief Maps an RGB OUTPUT index and component to a physical output index.
         *
         * @param index RGB OUTPUT index to map.
         * @param component RGB component to map.
         *
         * @return Physical output index corresponding to the RGB component.
         */
        size_t rgb_component_from_rgb(size_t index, RgbComponent component) override
        {
            return index * 3 + static_cast<uint8_t>(component);
        }

        private:
        static constexpr size_t register_count = DT_PROP(DT_NODELABEL(opendeck_outputs), shift_registers);

        gpio_dt_spec                        _data  = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_outputs), data_gpios);
        gpio_dt_spec                        _clock = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_outputs), clock_gpios);
        gpio_dt_spec                        _latch = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_outputs), latch_gpios);
        std::array<uint8_t, register_count> _state = {};

        /**
         * @brief Shifts the cached output state into the external registers.
         */
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

        /**
         * @brief Returns the number of complete RGB Outputs exposed by the driver.
         *
         * @return Number of RGB Outputs.
         */
        static constexpr size_t rgb_output_count()
        {
            return (register_count * 8) / 3;
        }
    };
}    // namespace opendeck::io::outputs
