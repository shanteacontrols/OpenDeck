/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/mdns/shared/deps.h"
#include "firmware/src/protocol/osc/shared/common.h"
#include "common/src/protocols/websockets/shared/common.h"

#include <array>

namespace opendeck::protocol::mdns
{
    /**
     * @brief In-memory DNS-SD service descriptors used by tests.
     */
    class ServicesTest : public Services
    {
        public:
        std::array<char, opendeck::common::protocols::mdns::NETWORK_NAME_SIZE> websockets_instance = {};
        std::array<char, opendeck::common::protocols::mdns::NETWORK_NAME_SIZE> osc_instance        = {};
        uint16_t                                                               websockets_port     = 0;
        uint16_t                                                               osc_port            = 0;

        /**
         * @brief Returns the mutable WebSockets DNS-SD service descriptor.
         *
         * @return WebSockets service descriptor.
         */
        opendeck::common::protocols::mdns::Service websockets() override
        {
            return {
                .instance     = websockets_instance,
                .port         = websockets_port,
                .service_port = opendeck::common::protocols::websockets::DEFAULT_PORT,
            };
        }

        /**
         * @brief Returns the mutable OSC DNS-SD service descriptor.
         *
         * @return OSC service descriptor.
         */
        opendeck::common::protocols::mdns::Service osc() override
        {
            return {
                .instance     = osc_instance,
                .port         = osc_port,
                .service_port = protocol::osc::DEFAULT_LISTEN_PORT,
            };
        }
    };
}    // namespace opendeck::protocol::mdns
