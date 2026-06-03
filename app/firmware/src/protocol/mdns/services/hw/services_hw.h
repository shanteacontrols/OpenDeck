/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/mdns/instance/impl/deps.h"

#include <vector>

namespace opendeck::firmware::protocol::mdns
{
    /**
     * @brief Zephyr DNS-SD service descriptors advertised by firmware mDNS.
     */
    class ServicesHw : public Services
    {
        public:
        /**
         * @brief Registers one DNS-SD service provider.
         *
         * @param provider Service provider owned by the mDNS builder.
         */
        static void register_service(ServiceProvider* provider);

        /**
         * @brief Returns DNS-SD service providers to advertise.
         *
         * @return DNS-SD service providers.
         */
        std::span<ServiceProvider* const> services() override;

        private:
        static inline std::vector<ServiceProvider*> providers;
    };
}    // namespace opendeck::firmware::protocol::mdns
