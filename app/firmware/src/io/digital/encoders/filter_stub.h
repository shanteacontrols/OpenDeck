/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

namespace opendeck::io::encoders
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
         * @param position Raw movement position.
         * @param filtered_position Output position after filtering.
         * @param sample_taken_time Timestamp associated with the sample.
         *
         * @return Always `false`.
         */
        bool is_filtered(size_t    index,
                         Position  position,
                         Position& filtered_position,
                         uint32_t  sample_taken_time) override
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

        /**
         * @brief Returns no movement timestamp.
         *
         * @param index Encoder index to query.
         *
         * @return Always `0`.
         */
        uint32_t last_movement_time(size_t index) override
        {
            return 0;
        }
    };
}    // namespace opendeck::io::encoders
