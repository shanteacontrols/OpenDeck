/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/mdns/shared/deps.h"
#include "common/src/mdns/shared/common.h"

#include <array>
#include <span>
#include <string_view>

namespace opendeck::mdns
{
    /**
     * @brief Shared OpenDeck mDNS operations.
     */
    class BaseMdns
    {
        public:
        /**
         * @brief Constructs shared mDNS support.
         *
         * @param hwa Platform hooks used by the shared mDNS code.
         */
        explicit BaseMdns(Hwa& hwa);

        /**
         * @brief Builds the default OpenDeck hostname.
         *
         * @return Hostname without `.local`, or an empty view on failure.
         */
        std::string_view default_hostname();

        /**
         * @brief Builds a hostname from stored custom hostname bytes.
         *
         * The stored name must be null-terminated, non-empty, contain only
         * letters, digits, and hyphens, and must not start or end with a hyphen.
         * Uppercase letters are converted to lowercase.
         *
         * @param hostname Stored null-terminated hostname bytes.
         *
         * @return Lowercase hostname without `.local`, or an empty view when invalid.
         */
        std::string_view custom_hostname(std::span<const uint8_t> hostname);

        /**
         * @brief Appends `.local` to the current hostname in-place.
         *
         * @return Full network name, or an empty view when it does not fit.
         */
        std::string_view network_name();

        /**
         * @brief Copies the current IPv4 address text.
         *
         * @param buffer Destination buffer.
         *
         * @return IPv4 address text, or an empty view if unavailable.
         */
        std::string_view ip_address(std::span<char> buffer);

        /**
         * @brief Registers the callback called when the local IPv4 address changes.
         *
         * @param callback Callback to store. Passing an empty callback disables notifications.
         */
        void register_ip_address_changed_callback(Hwa::IpAddressChangedCallback callback);

        /**
         * @brief Sets the network hostname used for `<hostname>.local`.
         *
         * @param hostname Hostname without the `.local` suffix.
         *
         * @return `true` when the hostname was accepted.
         */
        bool set_hostname(std::string_view hostname);

        /**
         * @brief Updates a static DNS-SD service record instance and port.
         *
         * @param instance DNS-SD instance name shown by discovery tools.
         * @param buffer DNS-SD record instance storage.
         * @param service_port DNS-SD record port storage.
         * @param port Service port in host byte order.
         *
         * @return `true` when the service instance was accepted.
         */
        bool advertise_service(std::string_view instance,
                               std::span<char>  buffer,
                               uint16_t&        service_port,
                               uint16_t         port);

        /**
         * @brief Updates a DNS-SD service descriptor instance and port.
         *
         * @param service Mutable DNS-SD service record fields.
         * @param instance DNS-SD instance name shown by discovery tools.
         *
         * @return `true` when the service instance was accepted.
         */
        bool advertise_service(Service service, std::string_view instance);

        private:
        Hwa&                                     _hwa;
        std::array<char, NETWORK_NAME_SIZE + 1U> _buffer = {};
        size_t                                   _size   = 0;

        /**
         * @brief Builds the default OpenDeck hostname using a prepared serial suffix.
         *
         * @param serial Lowercase hexadecimal serial suffix.
         *
         * @return Hostname without `.local`, or an empty view on failure.
         */
        std::string_view default_hostname(std::string_view serial);

        /**
         * @brief Appends the configured OpenDeck target name in hostname-safe form.
         *
         * @return `true` if the target name fit in the hostname buffer.
         */
        bool append_target();

        /**
         * @brief Appends the serial suffix, or the target UID fallback when unavailable.
         *
         * @param serial Lowercase hexadecimal serial suffix.
         *
         * @return `true` if the suffix fit in the hostname buffer.
         */
        bool append_serial(std::string_view serial);

        /**
         * @brief Appends the OpenDeck target UID as hexadecimal fallback suffix.
         *
         * @return `true` if the fallback suffix fit in the hostname buffer.
         */
        bool append_target_uid();

        /**
         * @brief Appends one byte as two lowercase hexadecimal characters.
         *
         * @param byte Byte to append.
         *
         * @return `true` if both characters fit in the hostname buffer.
         */
        bool append_hex_byte(uint8_t byte);

        /**
         * @brief Appends one character to the hostname buffer.
         *
         * @param character Character to append.
         *
         * @return `true` if the character fit in the hostname buffer.
         */
        bool append_char(char character);

        /**
         * @brief Clears the hostname buffer and resets the write position.
         */
        void reset();
    };
}    // namespace opendeck::mdns
