/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/mdns/shared/deps.h"
#include "common/src/mdns/instance/impl/mdns.h"

namespace opendeck::bootloader::mdns
{
    /**
     * @brief Bootloader mDNS discovery.
     */
    class Mdns
    {
        public:
        /**
         * @brief Constructs bootloader discovery around platform hooks.
         *
         * @param base_mdns Shared mDNS operations.
         * @param services DNS-SD service descriptors advertised by bootloader mDNS.
         */
        Mdns(opendeck::mdns::BaseMdns& base_mdns, Services& services);

        /**
         * @brief Configures hostname and advertises recovery service.
         *
         * @return `true` if discovery was configured.
         */
        bool init();

        private:
        opendeck::mdns::BaseMdns& _base_mdns;
        Services&                 _services;
    };
}    // namespace opendeck::bootloader::mdns
