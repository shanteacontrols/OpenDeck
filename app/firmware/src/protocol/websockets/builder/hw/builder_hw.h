/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/websockets/hwa/hw/hwa_hw.h"
#include "firmware/src/protocol/websockets/handler/builder/builder.h"
#include "firmware/src/protocol/websockets/instance/impl/websockets.h"

namespace opendeck::firmware::protocol::websockets
{
    /**
     * @brief Convenience builder that wires the WebSocket configuration endpoint.
     */
    class Builder
    {
        public:
        Builder()
            : _instance(_hwa)
        {}

        /**
         * @brief Returns the configured WebSockets protocol instance.
         *
         * @return WebSockets protocol instance.
         */
        WebSockets& instance()
        {
            return _instance;
        }

        private:
        handler::Builder _handlers;
        HwaHw            _hwa;
        WebSockets       _instance;
    };
}    // namespace opendeck::firmware::protocol::websockets
