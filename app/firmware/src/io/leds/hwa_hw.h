/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "remap.h"
#include "drivers/driver_base.h"

namespace io::leds
{
    /**
     * @brief Hardware-backed LED backend that proxies to a low-level LED driver.
     */
    class HwaHw : public Hwa
    {
        public:
        /**
         * @brief Constructs the backend around a low-level LED driver.
         *
         * @param driver Low-level LED driver implementation.
         */
        explicit HwaHw(DriverBase& driver)
            : _driver(driver)
        {}

        /**
         * @brief Flushes any pending state updates through the driver.
         */
        void update() override
        {
            _driver.update();
        }

        /**
         * @brief Sets the state of one LED output.
         *
         * @param index LED output index to update.
         * @param brightness Brightness value to apply.
         */
        void set_state(size_t index, Brightness brightness) override
        {
            _driver.set_state(Remap::physical(index), brightness);
        }

        /**
         * @brief Maps one LED output index to its RGB LED index.
         *
         * @param index LED output index to map.
         *
         * @return RGB LED index corresponding to the output.
         */
        size_t rgb_from_output(size_t index) override
        {
            return _driver.rgb_from_output(Remap::physical(index));
        }

        /**
         * @brief Maps an RGB LED index and component to a physical output index.
         *
         * @param index RGB LED index to map.
         * @param component RGB component to map.
         *
         * @return Logical output index corresponding to the RGB component.
         */
        size_t rgb_component_from_rgb(size_t index, RgbComponent component) override
        {
            return Remap::logical(_driver.rgb_component_from_rgb(index, component));
        }

        private:
        DriverBase& _driver;
    };
}    // namespace io::leds
