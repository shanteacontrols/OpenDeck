/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <zephyr/devicetree.h>

#include <cstddef>
#include <cstdint>

namespace io::leds
{
    enum class Brightness : uint8_t;
    enum class RgbComponent : uint8_t;

    /**
     * @brief Common interface implemented by low-level LED hardware drivers.
     */
    class DriverBase
    {
        public:
        virtual ~DriverBase() = default;

        /**
         * @brief Flushes any pending hardware state updates.
         */
        virtual void update() = 0;
        /**
         * @brief Sets the state of one physical LED output.
         *
         * @param index Output index to update.
         * @param brightness Brightness value to apply.
         */
        virtual void set_state(size_t index, Brightness brightness) = 0;
        /**
         * @brief Maps a physical output index to the corresponding RGB LED index.
         *
         * @param index Output index to map.
         *
         * @return RGB LED index corresponding to the output.
         */
        virtual size_t rgb_from_output(size_t index) = 0;
        /**
         * @brief Maps an RGB LED index and component to a physical output index.
         *
         * @param index RGB LED index to map.
         * @param component RGB component to map.
         *
         * @return Physical output index corresponding to the RGB component.
         */
        virtual size_t rgb_component_from_rgb(size_t index, RgbComponent component) = 0;
    };
}    // namespace io::leds
