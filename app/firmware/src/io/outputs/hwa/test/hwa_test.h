/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/shared/deps.h"

#include <gmock/gmock.h>

namespace opendeck::firmware::io::outputs
{
    /**
     * @brief Test output backend with mockable state updates.
     */
    class HwaTest : public Hwa
    {
        public:
        HwaTest() = default;

        MOCK_METHOD2(set_level, void(size_t index, uint8_t level));
    };
}    // namespace opendeck::firmware::io::outputs
