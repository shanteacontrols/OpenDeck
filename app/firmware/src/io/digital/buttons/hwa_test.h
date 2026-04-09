/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

#include <gmock/gmock.h>

namespace io::buttons
{
    /**
     * @brief Test button backend with mockable state access.
     */
    class HwaTest : public Hwa
    {
        public:
        HwaTest() = default;

        MOCK_METHOD0(init, bool());
        MOCK_METHOD1(state, std::optional<bool>(size_t index));
        MOCK_METHOD1(encoder_state, std::optional<uint8_t>(size_t index));

        /**
         * @brief Maps two buttons to one encoder in the test backend.
         *
         * @param index Button index to map.
         *
         * @return Button index divided by two.
         */
        size_t button_to_encoder_index(size_t index) override
        {
            return index / 2;
        }
    };
}    // namespace io::buttons
