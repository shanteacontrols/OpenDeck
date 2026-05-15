/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/drivers/driver_base.h"
#include "firmware/src/io/outputs/drivers/matrix_native_rows/count.h"

namespace opendeck::io::outputs
{
    /**
     * @brief OUTPUT driver for matrix-addressed outputs with native row handling.
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
         * @brief Maps a physical output index to the corresponding RGB OUTPUT index.
         *
         * @param index Output index to map.
         *
         * @return RGB OUTPUT index corresponding to the output.
         */
        size_t rgb_from_output(size_t index) override
        {
            const uint8_t columns  = DT_PROP(DT_NODELABEL(opendeck_outputs), columns);
            const uint8_t row      = index / columns;
            const uint8_t mod      = row % 3U;
            const uint8_t base_row = row - mod;
            const uint8_t column   = index % columns;
            const uint8_t result   = (base_row * columns) / 3U + column;
            return result < rgb_output_count() ? result : (rgb_output_count() ? rgb_output_count() - 1U : 0U);
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
            const uint8_t columns = DT_PROP(DT_NODELABEL(opendeck_outputs), columns);
            const uint8_t column  = index % columns;
            const uint8_t row     = (index / columns) * 3U;
            const uint8_t address = column + columns * row;
            return address + columns * static_cast<uint8_t>(component);
        }

        private:
        /**
         * @brief Returns the number of complete RGB Outputs exposed by the matrix.
         *
         * @return Number of RGB Outputs.
         */
        static constexpr size_t rgb_output_count()
        {
            return (DT_PROP(DT_NODELABEL(opendeck_outputs), rows) * DT_PROP(DT_NODELABEL(opendeck_outputs), columns)) / 3;
        }
    };
}    // namespace opendeck::io::outputs
