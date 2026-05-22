/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/mdns/shared/deps.h"
#include "firmware/src/protocol/osc/shared/common.h"
#include "firmware/src/protocol/webconfig/shared/common.h"

#include <array>

namespace opendeck::protocol::mdns
{
    /**
     * @brief In-memory DNS-SD service descriptors used by tests.
     */
    class ServicesTest : public Services
    {
        public:
        std::array<char, opendeck::mdns::NETWORK_NAME_SIZE> webconfig_instance = {};
        std::array<char, opendeck::mdns::NETWORK_NAME_SIZE> osc_instance       = {};
        uint16_t                                            webconfig_port     = 0;
        uint16_t                                            osc_port           = 0;

        /**
         * @brief Returns the mutable WebConfig DNS-SD service descriptor.
         *
         * @return WebConfig service descriptor.
         */
        opendeck::mdns::Service webconfig() override
        {
            return {
                .instance     = webconfig_instance,
                .port         = webconfig_port,
                .service_port = protocol::webconfig::DEFAULT_PORT,
            };
        }

        /**
         * @brief Returns the mutable OSC DNS-SD service descriptor.
         *
         * @return OSC service descriptor.
         */
        opendeck::mdns::Service osc() override
        {
            return {
                .instance     = osc_instance,
                .port         = osc_port,
                .service_port = protocol::osc::DEFAULT_LISTEN_PORT,
            };
        }
    };
}    // namespace opendeck::protocol::mdns
