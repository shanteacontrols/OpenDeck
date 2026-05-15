/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/database/database.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <string_view>
#include <sys/types.h>

namespace opendeck::protocol::mdns
{
    /**
     * @brief Database view used by mDNS for device-wide discovery settings.
     */
    using Database = database::User<database::Config::Section::Common>;

    /**
     * @brief Platform hooks used by the mDNS discovery backend.
     */
    class Hwa
    {
        public:
        using IpAddressChangedCallback = std::function<void()>;

        virtual ~Hwa() = default;

        /**
         * @brief Reads the hardware serial bytes used in the hostname suffix.
         *
         * @param buffer Destination buffer.
         *
         * @return Number of bytes copied, otherwise a negative errno value.
         */
        virtual ssize_t serial_number(std::span<uint8_t> buffer) = 0;

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
         * @brief Advertises the WebConfig DNS-SD service instance.
         *
         * @param instance DNS-SD instance name shown by discovery tools.
         *
         * @return `true` when the service instance was accepted.
         */
        virtual bool advertise_webconfig(std::string_view instance) = 0;

        /**
         * @brief Advertises the OSC DNS-SD service instance.
         *
         * @param instance DNS-SD instance name shown by discovery tools.
         *
         * @return `true` when the service instance was accepted.
         */
        virtual bool advertise_osc(std::string_view instance) = 0;
    };
}    // namespace opendeck::protocol::mdns
