/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "protocol/base.h"

namespace opendeck::protocol::osc
{
    /**
     * @brief Stub OSC subsystem used when OSC support is not enabled.
     */
    class Osc : public protocol::Base
    {
        public:
        Osc()           = default;
        ~Osc() override = default;

        /**
         * @brief Leaves the disabled OSC backend uninitialized.
         *
         * @return Always `false` because OSC is not available.
         */
        bool init() override
        {
            return false;
        }

        /**
         * @brief Deinitializes the disabled OSC backend.
         *
         * @return Always `true`.
         */
        bool deinit() override
        {
            return true;
        }
    };
}    // namespace opendeck::protocol::osc
