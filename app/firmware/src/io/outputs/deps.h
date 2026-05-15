/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/common.h"
#include "firmware/src/database/database.h"

namespace opendeck::io::outputs
{
    /**
     * @brief Database view used by the OUTPUT subsystem.
     */
    using Database = database::User<database::Config::Section::Outputs,
                                    database::Config::Section::Global>;

    /**
     * @brief Hardware abstraction used by the OUTPUT subsystem.
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
         * @brief Sets the state of one OUTPUT output.
         *
         * @param index OUTPUT output index to update.
         * @param brightness Brightness value to apply.
         */
        virtual void set_state(size_t index, Brightness brightness) = 0;

        /**
         * @brief Maps one OUTPUT output index to its RGB OUTPUT index.
         *
         * @param index OUTPUT output index to map.
         *
         * @return RGB OUTPUT index corresponding to the output.
         */
        virtual size_t rgb_from_output(size_t index) = 0;

        /**
         * @brief Maps an RGB OUTPUT index and component to a physical output index.
         *
         * @param index RGB OUTPUT index to map.
         * @param component RGB component to map.
         *
         * @return Physical output index corresponding to the RGB component.
         */
        virtual size_t rgb_component_from_rgb(size_t index, RgbComponent component) = 0;
    };
}    // namespace opendeck::io::outputs
