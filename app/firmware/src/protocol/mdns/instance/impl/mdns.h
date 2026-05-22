/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/mdns/shared/common.h"
#include "firmware/src/protocol/mdns/shared/deps.h"
#include "firmware/src/protocol/base.h"
#include "firmware/src/system/config.h"
#include "firmware/src/threads.h"
#include "common/src/mdns/instance/impl/mdns.h"

#include "zlibs/utils/misc/kwork_delayable.h"

#include <array>
#include <optional>
#include <string_view>

namespace opendeck::protocol::mdns
{
    /**
     * @brief mDNS/DNS-SD discovery backend.
     */
    class Mdns : public protocol::Base
    {
        public:
        /**
         * @brief Constructs the discovery backend.
         *
         * @param base_mdns Shared mDNS operations.
         * @param services DNS-SD service descriptors advertised by firmware mDNS.
         * @param database Device-wide database settings used by mDNS.
         */
        Mdns(opendeck::mdns::BaseMdns& base_mdns, Services& services, Database& database);

        /**
         * @brief Configures the mDNS hostname and WebConfig service advertisement.
         *
         * @return `true` if discovery was configured, otherwise `false`.
         */
        bool init() override;

        /**
         * @brief Leaves Zephyr's mDNS responder running.
         *
         * @return Always `true`.
         */
        bool deinit() override;

        private:
        opendeck::mdns::BaseMdns&                                 _base_mdns;
        Services&                                                 _services;
        Database&                                                 _database;
        zlibs::utils::misc::KworkDelayable                        _network_identity_work;
        std::array<char, opendeck::mdns::IPV4_ADDRESS_SIZE>       _ip_address             = {};
        std::array<uint8_t, opendeck::mdns::CUSTOM_HOSTNAME_SIZE> _custom_hostname        = {};
        bool                                                      _custom_hostname_loaded = false;

        /**
         * @brief Builds the hostname used by mDNS.
         *
         * @return Hostname without `.local`, or an empty view on failure.
         */
        std::string_view make_hostname();

        /**
         * @brief Appends the stored custom hostname if one is configured.
         *
         * @return Custom hostname, or an empty view if none is configured.
         */
        std::string_view append_custom_hostname();

        /**
         * @brief Loads the custom hostname bytes from the database.
         */
        void load_custom_hostname();

        /**
         * @brief Publishes the network identity built by mDNS.
         *
         * @param identity Full `.local` name and current IPv4 address.
         */
        void publish_network_identity(const opendeck::mdns::NetworkIdentity& identity);

        /**
         * @brief Schedules network identity publishing outside the caller's stack context.
         */
        void schedule_network_identity_publish();

        /**
         * @brief Republishes network identity after the local IP address changes.
         */
        void handle_ip_address_changed();

        /**
         * @brief Reads one mDNS SysEx configuration value.
         *
         * @param section Global SysEx configuration section.
         * @param index Entry index within `section`.
         * @param value Output storage for the returned value.
         *
         * @return Optional status code, or empty if the section is not owned by mDNS.
         */
        std::optional<uint8_t> sys_config_get(sys::Config::Section::Global section, size_t index, uint16_t& value);

        /**
         * @brief Writes one mDNS SysEx configuration value.
         *
         * @param section Global SysEx configuration section.
         * @param index Entry index within `section`.
         * @param value Value to store.
         *
         * @return Optional status code, or empty if the section is not owned by mDNS.
         */
        std::optional<uint8_t> sys_config_set(sys::Config::Section::Global section, size_t index, uint16_t value);
    };
}    // namespace opendeck::protocol::mdns
