/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/encoders/instance/impl/deps.h"

namespace opendeck::firmware::io::encoders
{
    /**
     * @brief Stub encoder filter that rejects every movement sample.
     */
    class FilterStub : public Filter
    {
        public:
        FilterStub() = default;

        /**
         * @brief Rejects every encoder movement sample.
         *
         * @param index Encoder index being processed.
         * @param pair_state Current two-bit encoder pair state.
         * @param filtered_position Output position after filtering.
         * @param movement_elapsed_time Time since the previous movement, or `std::nullopt` when none exists.
         *
         * @return Always `false`.
         */
        bool is_filtered(size_t                  index,
                         uint8_t                 pair_state,
                         Position&               filtered_position,
                         std::optional<uint32_t> movement_elapsed_time) override
        {
            return false;
        }

        /**
         * @brief Resets no state because the stub filter keeps none.
         *
         * @param index Encoder index to reset.
         */
        void reset(size_t index) override
        {
        }
    };
}    // namespace opendeck::firmware::io::encoders
