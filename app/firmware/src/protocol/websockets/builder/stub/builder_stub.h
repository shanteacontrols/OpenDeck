/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/websockets/instance/stub/websockets_stub.h"

namespace opendeck::firmware::protocol::websockets
{
    /**
     * @brief Stub builder that wires WebSocket configuration to a no-op backend.
     */
    class Builder
    {
        public:
        Builder() = default;

        /**
         * @brief Returns the disabled WebSockets protocol instance.
         *
         * @return Stub WebSockets protocol instance.
         */
        WebSockets& instance()
        {
            return _instance;
        }

        private:
        WebSockets _instance;
    };
}    // namespace opendeck::firmware::protocol::websockets
