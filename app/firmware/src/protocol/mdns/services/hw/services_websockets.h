/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/mdns/services/hw/services_hw.h"
#include "firmware/src/protocol/mdns/instance/impl/deps.h"

namespace opendeck::firmware::protocol::mdns
{
    /**
     * @brief DNS-SD service provider for the WebSockets endpoint.
     */
    class WebSocketsService : public ServiceProvider
    {
        public:
        /**
         * @brief Constructs the WebSockets service provider and registers it with mDNS services.
         */
        WebSocketsService();

        /**
         * @brief Returns the WebSockets DNS-SD service descriptor.
         *
         * @return WebSockets DNS-SD service descriptor.
         */
        opendeck::common::protocols::mdns::Service service() override;
    };
}    // namespace opendeck::firmware::protocol::mdns
