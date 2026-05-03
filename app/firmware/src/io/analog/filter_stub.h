/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

namespace opendeck::io::analog
{
    /**
     * @brief Stub analog filter that never forwards readings.
     */
    class FilterStub : public Filter
    {
        public:
        FilterStub() = default;

        /**
         * @brief Rejects every reading without modifying it.
         *
         * @param index Analog input index being processed.
         * @param descriptor Runtime descriptor containing the reading.
         *
         * @return Always `false`.
         */
        bool is_filtered(size_t index, Descriptor& descriptor) override
        {
            return false;
        }

        /**
         * @brief Resets no state because the stub keeps none.
         *
         * @param index Analog input index to reset.
         */
        void reset(size_t index) override
        {
        }
    };
}    // namespace opendeck::io::analog
