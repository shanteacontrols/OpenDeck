/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/websockets/instance/stub/websockets_stub.h"

namespace opendeck::protocol::websockets
{
    /**
     * @brief Test builder that keeps WebSockets out of host-test network stacks.
     */
    class Builder
    {
        public:
        Builder() = default;

        /**
         * @brief Returns the test WebSockets protocol instance.
         *
         * @return Stub WebSockets protocol instance.
         */
        WebSockets& instance()
        {
            return _instance;
        }

        WebSockets _instance;
    };
}    // namespace opendeck::protocol::websockets
