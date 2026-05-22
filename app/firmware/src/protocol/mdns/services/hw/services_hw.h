/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/mdns/shared/deps.h"

namespace opendeck::protocol::mdns
{
    /**
     * @brief Zephyr DNS-SD service descriptors advertised by firmware mDNS.
     */
    class ServicesHw : public Services
    {
        public:
        /**
         * @brief Returns the mutable WebSockets DNS-SD service descriptor.
         *
         * @return WebSockets service descriptor.
         */
        opendeck::common::protocols::mdns::Service websockets() override;

        /**
         * @brief Returns the mutable OSC DNS-SD service descriptor.
         *
         * @return OSC service descriptor.
         */
        opendeck::common::protocols::mdns::Service osc() override;
    };
}    // namespace opendeck::protocol::mdns
