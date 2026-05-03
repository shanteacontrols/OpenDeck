/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

namespace opendeck::io::buttons
{
    /**
     * @brief Test button filter that accepts every button reading.
     */
    class FilterTest : public Filter
    {
        public:
        FilterTest() = default;

        /**
         * @brief Accepts every button reading.
         *
         * @param index Button index being processed.
         * @param state Input reading being filtered.
         *
         * @return Always `true`.
         */
        bool is_filtered(size_t index, bool& state) override
        {
            return true;
        }
    };
}    // namespace opendeck::io::buttons
