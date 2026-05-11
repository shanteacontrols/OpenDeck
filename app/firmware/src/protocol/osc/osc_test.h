/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "protocol/base.h"

namespace opendeck::protocol::osc
{
    /**
     * @brief Test OSC backend used by host tests when OSC support is enabled.
     */
    class Osc : public protocol::Base
    {
        public:
        Osc()           = default;
        ~Osc() override = default;

        /**
         * @brief Marks the test backend initialized.
         *
         * @return Always `true`.
         */
        bool init() override
        {
            _initialized = true;
            return true;
        }

        /**
         * @brief Marks the test backend deinitialized.
         *
         * @return Always `true`.
         */
        bool deinit() override
        {
            _initialized = false;
            return true;
        }

        /**
         * @brief Returns whether the test backend is initialized.
         *
         * @return `true` after init and before deinit.
         */
        bool initialized() const
        {
            return _initialized;
        }

        private:
        bool _initialized = false;
    };
}    // namespace opendeck::protocol::osc
