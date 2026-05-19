/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/analog/shared/common.h"

#include <optional>

namespace opendeck::io::analog
{
    /**
     * @brief Shared cache of the most recent analog scan frame.
     */
    class FrameStore
    {
        public:
        /**
         * @brief Stores a new analog frame and marks it valid.
         *
         * @param frame Frame to cache.
         */
        void set_frame(const Frame& frame)
        {
            _frame       = frame;
            _frame_valid = true;
        }

        /**
         * @brief Invalidates the cached frame.
         */
        void clear()
        {
            _frame_valid = false;
        }

        /**
         * @brief Returns the cached value of one logical analog input.
         *
         * @param index Logical analog input index to query.
         *
         * @return Cached value, or `std::nullopt` when the frame is invalid or the index is out of range.
         */
        std::optional<uint16_t> value(size_t index) const
        {
            if (!_frame_valid || (index >= _frame.size()))
            {
                return {};
            }

            return _frame[index];
        }

        private:
        Frame _frame       = {};
        bool  _frame_valid = false;
    };
}    // namespace opendeck::io::analog
