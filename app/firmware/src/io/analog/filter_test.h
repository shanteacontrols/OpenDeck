/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/analog/deps.h"

namespace opendeck::io::analog
{
    /**
     * @brief Test analog filter that forwards every reading.
     */
    class FilterTest : public Filter
    {
        public:
        FilterTest() = default;

        /**
         * @brief Accepts every reading without modifying it.
         *
         * @param index Analog input index being processed.
         * @param descriptor Runtime descriptor containing the reading.
         *
         * @return Always `true`.
         */
        bool is_filtered(size_t index, Descriptor& descriptor) override
        {
            return true;
        }

        /**
         * @brief Resets no state because the test filter keeps none.
         *
         * @param index Analog input index to reset.
         */
        void reset(size_t index) override
        {
        }
    };
}    // namespace opendeck::io::analog
