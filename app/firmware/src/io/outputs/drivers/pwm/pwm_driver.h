/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/shared/deps.h"

#include <zephyr/drivers/pwm.h>

#include <array>

#define OPENDECK_OUTPUT_PWM_ENTRY(index, node_id) PWM_DT_SPEC_GET_BY_IDX(node_id, index)

namespace opendeck::io::outputs
{
    /**
     * @brief Output driver that drives native PWM outputs directly.
     */
    class Driver : public Hwa
    {
        public:
        /**
         * @brief Stops all configured PWM outputs at construction time.
         */
        Driver()
        {
            for (const auto& pwm : _pwms)
            {
                if (pwm_is_ready_dt(&pwm))
                {
                    pwm_set_pulse_dt(&pwm, 0);
                }
            }
        }

        /**
         * @brief Sets one PWM-backed output.
         *
         * @param index Output index to update.
         * @param level Output level percentage in the range [0, 100].
         */
        void set_level(size_t index, uint8_t level) override
        {
            if (index >= _pwms.size())
            {
                return;
            }

            const auto& pwm   = _pwms[index];
            const auto  pulse = static_cast<uint32_t>((static_cast<uint64_t>(pwm.period) * level) / OUTPUT_LEVEL_MAX);

            pwm_set_pulse_dt(&pwm, pulse);
        }

        private:
        static constexpr size_t OUTPUT_COUNT = CONFIG_PROJECT_TARGET_OUTPUT_PHYSICAL_COUNT;

        std::array<pwm_dt_spec, OUTPUT_COUNT> _pwms = { { LISTIFY(CONFIG_PROJECT_TARGET_OUTPUT_PHYSICAL_COUNT, OPENDECK_OUTPUT_PWM_ENTRY, (, ), DT_NODELABEL(opendeck_outputs)) } };
    };
}    // namespace opendeck::io::outputs

#undef OPENDECK_OUTPUT_PWM_ENTRY
