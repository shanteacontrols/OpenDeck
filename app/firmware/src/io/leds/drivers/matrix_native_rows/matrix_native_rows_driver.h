/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "../driver_base.h"
#include "count.h"

namespace opendeck::io::leds
{
    /**
     * @brief LED driver for matrix-addressed outputs with native row handling.
     */
    class Driver : public DriverBase
    {
        public:
        /**
         * @brief Flushes no additional state because this driver exposes addressing helpers only.
         */
        void update() override
        {
        }

        /**
         * @brief Ignores direct state writes in this addressing-only driver.
         *
         * @param index Output index to update.
         * @param brightness Brightness value that would be applied.
         */
        void set_state(size_t /*index*/, Brightness /*brightness*/) override
        {
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
            const uint8_t columns  = DT_PROP(DT_NODELABEL(opendeck_leds), columns);
            const uint8_t row      = index / columns;
            const uint8_t mod      = row % 3U;
            const uint8_t base_row = row - mod;
            const uint8_t column   = index % columns;
            const uint8_t result   = (base_row * columns) / 3U + column;
            return result < rgb_led_count() ? result : (rgb_led_count() ? rgb_led_count() - 1U : 0U);
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
            const uint8_t columns = DT_PROP(DT_NODELABEL(opendeck_leds), columns);
            const uint8_t column  = index % columns;
            const uint8_t row     = (index / columns) * 3U;
            const uint8_t address = column + columns * row;
            return address + columns * static_cast<uint8_t>(component);
        }

        private:
        /**
         * @brief Returns the number of complete RGB LEDs exposed by the matrix.
         *
         * @return Number of RGB LEDs.
         */
        static constexpr size_t rgb_led_count()
        {
            return (DT_PROP(DT_NODELABEL(opendeck_leds), rows) * DT_PROP(DT_NODELABEL(opendeck_leds), columns)) / 3;
        }
    };
}    // namespace opendeck::io::leds
