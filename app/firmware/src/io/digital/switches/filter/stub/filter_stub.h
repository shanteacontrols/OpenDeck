/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/switches/shared/deps.h"

namespace opendeck::io::switches
{
    /**
     * @brief Stub switch filter that rejects every switch reading.
     */
    class FilterTest : public Filter
    {
        public:
        FilterTest() = default;

        /**
         * @brief Rejects every switch reading.
         *
         * @param index Switch index being processed.
         * @param state Input reading being filtered.
         *
         * @return Always `false`.
         */
        bool is_filtered(size_t index, bool& state) override
        {
            return false;
        }
    };
}    // namespace opendeck::io::switches
