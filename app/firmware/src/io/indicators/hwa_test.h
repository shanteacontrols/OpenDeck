/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

namespace io::indicators
{
    /**
     * @brief Test indicator backend that accepts requests without side effects.
     */
    class HwaTest : public Hwa
    {
        public:
        /**
         * @brief Initializes the test backend.
         *
         * @return Always `true`.
         */
        bool init() override
        {
            return true;
        }

        /**
         * @brief Ignores indicator-enable requests in tests.
         *
         * @param type Indicator to enable.
         */
        void on(Type type) override
        {
        }

        /**
         * @brief Ignores indicator-disable requests in tests.
         *
         * @param type Indicator to disable.
         */
        void off(Type type) override
        {
        }
    };
}    // namespace io::indicators
