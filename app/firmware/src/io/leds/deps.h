/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common.h"

#include "database/database.h"

namespace io::leds
{
    /**
     * @brief Database view used by the LED subsystem.
     */
    using Database = database::User<database::Config::Section::Leds,
                                    database::Config::Section::Global>;

    /**
     * @brief Hardware abstraction used by the LED subsystem.
     */
    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        /**
         * @brief Flushes any pending hardware state updates.
         */
        virtual void update() = 0;
        /**
         * @brief Sets the state of one LED output.
         *
         * @param index LED output index to update.
         * @param brightness Brightness value to apply.
         */
        virtual void set_state(size_t index, Brightness brightness) = 0;
        /**
         * @brief Maps one LED output index to its RGB LED index.
         *
         * @param index LED output index to map.
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
