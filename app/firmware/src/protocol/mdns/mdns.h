/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common.h"
#include "deps.h"
#include "protocol/base.h"
#include "system/config.h"

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
         * @param hwa Platform hooks used for hostname and DNS-SD setup.
         * @param database Device-wide database settings used by mDNS.
         */
        Mdns(Hwa& hwa, Database& database);

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
        static constexpr size_t SERIAL_BUFFER_SIZE = 16;

        Hwa&                                         _hwa;
        Database&                                    _database;
        std::array<char, NETWORK_NAME_SIZE>          _hostname               = {};
        std::array<char, IPV4_ADDRESS_SIZE>          _ip_address             = {};
        std::array<uint8_t, CUSTOM_HOSTNAME_DB_SIZE> _custom_hostname        = {};
        size_t                                       _hostname_size          = 0;
        bool                                         _custom_hostname_loaded = false;

        /**
         * @brief Builds the hostname used by mDNS.
         *
         * @return Hostname without `.local`, or an empty view on failure.
         */
        std::string_view make_hostname();

        /**
         * @brief Appends the stored custom hostname if one is configured.
         *
         * @return `true` if a valid custom hostname was appended.
         */
        bool append_custom_hostname();

        /**
         * @brief Loads the custom hostname bytes from the database.
         */
        void load_custom_hostname();

        /**
         * @brief Builds the `.local` name published to other protocols.
         *
         * @return Full mDNS name, or an empty view if it does not fit.
         */
        std::string_view make_network_name();

        /**
         * @brief Publishes the network identity built by mDNS.
         *
         * @param name Full `.local` name advertised by mDNS.
         */
        void publish_network_identity(std::string_view name);

        /**
         * @brief Republishes network identity after the local IP address changes.
         */
        void handle_ip_address_changed();

        /**
         * @brief Appends the OpenDeck target name as a hostname-safe label.
         */
        bool append_target();

        /**
         * @brief Appends a hardware serial suffix.
         */
        bool append_serial();

        /**
         * @brief Appends one byte as two lowercase hex characters.
         *
         * @param byte Byte to append.
         *
         * @return `true` if the byte was appended.
         */
        bool append_hex_byte(uint8_t byte);

        /**
         * @brief Appends one character if there is room.
         *
         * @param character Character to append.
         *
         * @return `true` if the character was appended.
         */
        bool append_char(char character);

        /**
         * @brief Clears the hostname assembly buffer.
         */
        void reset_hostname();

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
