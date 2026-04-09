/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

namespace io::indicators
{
    /**
     * @brief Stub indicator backend that accepts requests without driving hardware.
     */
    class HwaStub : public Hwa
    {
        public:
        /**
         * @brief Initializes the stub backend.
         *
         * @return Always `true`.
         */
        bool init() override
        {
            return true;
        }

        /**
         * @brief Ignores indicator-enable requests.
         *
         * @param type Indicator to enable.
         */
        void on([[maybe_unused]] Type type) override
        {
        }

        /**
         * @brief Ignores indicator-disable requests.
         *
         * @param type Indicator to disable.
         */
        void off([[maybe_unused]] Type type) override
        {
        }
    };
}    // namespace io::indicators
