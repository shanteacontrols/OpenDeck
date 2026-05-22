/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/mcu/hwa/test/hwa_test.h"

namespace opendeck::common::mcu
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
        opendeck::common::mcu::HwaTest& instance()
        {
            return _hwa;
        }

        opendeck::common::mcu::HwaTest _hwa;
    };
}    // namespace opendeck::common::mcu
