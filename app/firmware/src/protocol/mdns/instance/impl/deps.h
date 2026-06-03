/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/protocols/mdns/shared/common.h"
#include "common/src/protocols/mdns/shared/deps.h"
#include "firmware/src/database/instance/impl/database.h"

#include <span>

namespace opendeck::firmware::protocol::mdns
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
     * @brief Produces one DNS-SD service descriptor advertised by mDNS.
     */
    class ServiceProvider
    {
        public:
        virtual ~ServiceProvider() = default;

        /**
         * @brief Returns the DNS-SD service descriptor.
         *
         * @return DNS-SD service descriptor.
         */
        virtual opendeck::common::protocols::mdns::Service service() = 0;
    };

    /**
     * @brief DNS-SD services advertised by the firmware mDNS backend.
     */
    class Services
    {
        public:
        virtual ~Services() = default;

        /**
         * @brief Returns DNS-SD service providers to advertise.
         *
         * @return DNS-SD service providers.
         */
        virtual std::span<ServiceProvider* const> services() = 0;
    };
}    // namespace opendeck::firmware::protocol::mdns
