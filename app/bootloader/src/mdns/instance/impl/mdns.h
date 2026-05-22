/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/mdns/shared/deps.h"
#include "common/src/mdns/instance/impl/mdns.h"

#include <array>

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
        opendeck::mdns::BaseMdns&                           _base_mdns;
        Services&                                           _services;
        std::array<char, opendeck::mdns::IPV4_ADDRESS_SIZE> _ip_address = {};

        /**
         * @brief Logs the bootloader mDNS network identity.
         *
         * @param identity Full `.local` name and current IPv4 address.
         */
        void publish_network_identity(const opendeck::mdns::NetworkIdentity& identity);

        /**
         * @brief Republishes network identity after the local IP address changes.
         */
        void handle_ip_address_changed();
    };
}    // namespace opendeck::bootloader::mdns
