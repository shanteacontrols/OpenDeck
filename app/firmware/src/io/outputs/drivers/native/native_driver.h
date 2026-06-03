/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/instance/impl/deps.h"

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include <array>

#define OPENDECK_OUTPUT_NATIVE_GPIO_ENTRY(index, node_id) GPIO_DT_SPEC_GET_BY_IDX(node_id, native_gpios, index)

namespace opendeck::firmware::io::outputs
{
    /**
     * @brief Output driver that drives native GPIO outputs directly.
     */
    class Driver : public Hwa
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
         * @brief Sets one GPIO-backed output.
         *
         * @param index Output index to update.
         * @param level Output level percentage in the range [0, 100].
         */
        void set_level(size_t index, uint8_t level) override
        {
            if (index >= _gpios.size())
            {
                return;
            }

            gpio_pin_set_dt(&_gpios[index], level > OUTPUT_LEVEL_MIN);
        }

        private:
        static constexpr size_t OUTPUT_COUNT = CONFIG_PROJECT_TARGET_OUTPUT_PHYSICAL_COUNT;

        std::array<gpio_dt_spec, OUTPUT_COUNT> _gpios = { { LISTIFY(CONFIG_PROJECT_TARGET_OUTPUT_PHYSICAL_COUNT, OPENDECK_OUTPUT_NATIVE_GPIO_ENTRY, (, ), DT_NODELABEL(opendeck_outputs)) } };
    };
}    // namespace opendeck::firmware::io::outputs

#undef OPENDECK_OUTPUT_NATIVE_GPIO_ENTRY
