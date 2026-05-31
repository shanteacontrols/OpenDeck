/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/protocols/websockets/handler/handler.h"

namespace opendeck::protocol::websockets::osc
{
    /**
     * @brief Queues outbound OSC events as WebSockets frames.
     */
    class OscHandler : public opendeck::common::protocols::websockets::Handler
    {
        public:
        /**
         * @brief Subscribes to OSC event sources.
         *
         * @param endpoint WebSockets endpoint that queues encoded OSC packets.
         */
        void init(opendeck::common::protocols::websockets::HandlerEndpoint& endpoint) override;
    };
}    // namespace opendeck::protocol::websockets::osc
