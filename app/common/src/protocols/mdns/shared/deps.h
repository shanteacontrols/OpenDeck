/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <functional>
#include <span>
#include <string_view>

namespace opendeck::common::protocols::mdns
{
    /**
     * @brief Platform hooks used by shared OpenDeck mDNS code.
     */
    class Hwa
    {
        public:
        using IpAddressChangedCallback = std::function<void()>;

        virtual ~Hwa() = default;

        /**
         * @brief Returns the hardware serial text used in the hostname suffix.
         *
         * @return Serial suffix text, or an empty view when unavailable.
         */
        virtual std::string_view serial_number() = 0;

        /**
         * @brief Copies the current IPv4 address text.
         *
         * @param buffer Destination buffer.
         *
         * @return IPv4 address text, or an empty view if unavailable.
         */
        virtual std::string_view ip_address(std::span<char> buffer) = 0;

        /**
         * @brief Registers the callback called when the local IPv4 address changes.
         *
         * @param callback Callback to store. Passing an empty callback disables notifications.
         */
        virtual void register_ip_address_changed_callback(IpAddressChangedCallback callback) = 0;

        /**
         * @brief Sets the network hostname used for `<hostname>.local`.
         *
         * @param hostname Hostname without the `.local` suffix.
         *
         * @return `true` when the hostname was accepted.
         */
        virtual bool set_hostname(std::string_view hostname) = 0;

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
        virtual bool advertise_service(std::string_view instance,
                                       std::span<char>  buffer,
                                       uint16_t&        service_port,
                                       uint16_t         port) = 0;
    };
}    // namespace opendeck::common::protocols::mdns
