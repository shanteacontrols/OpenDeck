/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

#include <gmock/gmock.h>

namespace opendeck::io::encoders
{
    /**
     * @brief Test encoder backend with mockable state access.
     */
    class HwaTest : public Hwa
    {
        public:
        HwaTest() = default;

        MOCK_METHOD1(state, std::optional<uint8_t>(size_t index));
    };
}    // namespace opendeck::io::encoders
