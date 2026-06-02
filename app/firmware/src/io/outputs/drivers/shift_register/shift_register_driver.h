/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/shared/deps.h"

#include "zlibs/utils/misc/bit.h"

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include <array>

namespace opendeck::firmware::io::outputs
{
    /**
     * @brief Output driver that shifts output state through one or more shift registers.
     */
    class Driver : public Hwa
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
         * @brief Sets one shift-register-driven output and flushes the register state.
         *
         * @param index Output index to update.
         * @param level Output level percentage in the range [0, 100].
         */
        void set_level(size_t index, uint8_t level) override
        {
            if (index >= register_count * 8)
            {
                return;
            }

            const size_t byte = index / 8;
            const size_t bit  = index % 8;

            zlibs::utils::misc::bit_write(_state[byte], bit, level > OUTPUT_LEVEL_MIN);
            flush();
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
    };
}    // namespace opendeck::firmware::io::outputs
