/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "protocol/base.h"

namespace opendeck::protocol::webconfig
{
    /**
     * @brief Stub WebSocket configuration endpoint used when support is disabled.
     */
    class WebConfig : public protocol::Base
    {
        public:
        WebConfig()           = default;
        ~WebConfig() override = default;

        bool init() override
        {
            return true;
        }

        bool deinit() override
        {
            return true;
        }
    };
}    // namespace opendeck::protocol::webconfig
