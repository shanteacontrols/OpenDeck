/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "hwa_test.h"

namespace opendeck::mcu
{
    /**
     * @brief Test MCU builder.
     */
    class Builder
    {
        public:
        Builder() = default;

        /**
         * @brief Returns the configured test MCU services.
         *
         * @return Test MCU service backend.
         */
        HwaTest& instance()
        {
            return _hwa;
        }

        HwaTest _hwa;
    };
}    // namespace opendeck::mcu
