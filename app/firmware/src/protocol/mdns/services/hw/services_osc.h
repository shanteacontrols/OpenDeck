/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/mdns/services/hw/services_hw.h"
#include "firmware/src/protocol/mdns/shared/deps.h"

namespace opendeck::firmware::protocol::mdns
{
    /**
     * @brief DNS-SD service provider for the OSC endpoint.
     */
    class OscService : public ServiceProvider
    {
        public:
        /**
         * @brief Constructs the OSC service provider and registers it with mDNS services.
         */
        OscService();

        /**
         * @brief Returns the OSC DNS-SD service descriptor.
         *
         * @return OSC DNS-SD service descriptor.
         */
        opendeck::common::protocols::mdns::Service service() override;
    };
}    // namespace opendeck::firmware::protocol::mdns
