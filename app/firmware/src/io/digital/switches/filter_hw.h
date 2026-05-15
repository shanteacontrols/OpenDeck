/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/switches/deps.h"

namespace opendeck::io::switches
{
    /**
     * @brief Debouncing filter for digital switch readings.
     */
    class FilterHw : public Filter
    {
        public:
        FilterHw() = default;

        /** @brief Bit pattern indicating a fully debounced pressed state. */
        static constexpr uint8_t DEBOUNCE_STATE_ACTIVE = 0xFF;

        /**
         * @brief Debounces one switch reading using an 8-sample shift register.
         *
         * @param index Switch index being processed.
         * @param state Input reading updated with the debounced state when stable.
         *
         * @return `true` when the state is debounced, otherwise `false`.
         *
         * @note The implementation assumes consecutive readings arrive 1 ms apart,
         *       yielding an effective debounce interval of 8 ms.
         */
        bool is_filtered(size_t index, bool& state) override
        {
            if (index >= sizeof(_debounce_state))
            {
                return true;
            }

            _debounce_state[index] <<= 1;
            _debounce_state[index] |= static_cast<uint8_t>(state);

            if (_debounce_state[index] == DEBOUNCE_STATE_ACTIVE)
            {
                state = 1;
            }
            else if (_debounce_state[index] == 0)
            {
                state = 0;
            }
            else
            {
                // not debounced yet
                return false;
            }

            return true;
        }

        private:
        uint8_t _debounce_state[Collection::size(GroupDigitalInputs)] = {};
    };
}    // namespace opendeck::io::switches
