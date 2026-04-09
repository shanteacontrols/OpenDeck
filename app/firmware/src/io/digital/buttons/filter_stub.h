/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

namespace io::buttons
{
    /**
     * @brief Stub button filter that rejects every button reading.
     */
    class FilterTest : public Filter
    {
        public:
        FilterTest() = default;

        /**
         * @brief Rejects every button reading.
         *
         * @param index Button index being processed.
         * @param state Input reading being filtered.
         *
         * @return Always `false`.
         */
        bool is_filtered(size_t index, bool& state) override
        {
            return false;
        }
    };
}    // namespace io::buttons
