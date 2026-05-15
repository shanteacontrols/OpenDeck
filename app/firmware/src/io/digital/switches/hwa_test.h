/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/switches/deps.h"

#include <gmock/gmock.h>

namespace opendeck::io::switches
{
    /**
     * @brief Test switch backend with mockable state access.
     */
    class HwaTest : public Hwa
    {
        public:
        HwaTest() = default;

        MOCK_METHOD0(init, bool());
        MOCK_METHOD1(state, std::optional<bool>(size_t index));
        MOCK_METHOD1(encoder_state, std::optional<uint8_t>(size_t index));

        /**
         * @brief Maps two switches to one encoder in the test backend.
         *
         * @param index Switch index to map.
         *
         * @return Switch index divided by two.
         */
        size_t switch_to_encoder_index(size_t index) override
        {
            return index / 2;
        }
    };
}    // namespace opendeck::io::switches
