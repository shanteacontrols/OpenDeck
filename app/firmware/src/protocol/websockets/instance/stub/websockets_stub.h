/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/base.h"
#include "common/src/protocols/websockets/shared/deps.h"

#include <cerrno>

namespace opendeck::protocol::websockets
{
    /**
     * @brief Stub WebSocket configuration endpoint used when support is disabled.
     */
    class WebSockets : public protocol::Base, public opendeck::common::protocols::websockets::Endpoint
    {
        public:
        WebSockets()           = default;
        ~WebSockets() override = default;

        bool init() override
        {
            return true;
        }

        bool deinit() override
        {
            return true;
        }

        int accept_client([[maybe_unused]] int socket) override
        {
            return -ENOTSUP;
        }
    };
}    // namespace opendeck::protocol::websockets
