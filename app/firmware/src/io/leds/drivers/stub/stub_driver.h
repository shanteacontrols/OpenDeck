/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "../driver_base.h"

namespace io::leds
{
    /**
     * @brief No-op LED driver used for touchscreen-only targets.
     */
    class Driver : public DriverBase
    {
        public:
        void update() override
        {
        }

        void set_state(size_t, Brightness) override
        {
        }

        size_t rgb_from_output(size_t) override
        {
            return 0;
        }

        size_t rgb_component_from_rgb(size_t, RgbComponent) override
        {
            return 0;
        }
    };
}    // namespace io::leds
