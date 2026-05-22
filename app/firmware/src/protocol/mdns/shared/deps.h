/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/protocols/mdns/shared/common.h"
#include "common/src/protocols/mdns/shared/deps.h"
#include "firmware/src/database/instance/impl/database.h"

namespace opendeck::protocol::mdns
{
    /**
     * @brief Database view used by mDNS for device-wide discovery settings.
     */
    using Database = database::User<database::Config::Section::Common>;

    /**
     * @brief Platform hooks used by the shared mDNS backend.
     */
    using Hwa = opendeck::common::protocols::mdns::Hwa;

    /**
     * @brief DNS-SD services advertised by the firmware mDNS backend.
     */
    class Services
    {
        public:
        virtual ~Services() = default;

        /**
         * @brief Returns the mutable WebSockets DNS-SD service descriptor.
         *
         * @return WebSockets service descriptor.
         */
        virtual opendeck::common::protocols::mdns::Service websockets() = 0;

        /**
         * @brief Returns the mutable OSC DNS-SD service descriptor.
         *
         * @return OSC service descriptor.
         */
        virtual opendeck::common::protocols::mdns::Service osc() = 0;
    };
}    // namespace opendeck::protocol::mdns
