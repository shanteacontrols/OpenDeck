/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/mdns/deps.h"

#include <algorithm>
#include <array>
#include <string>
#include <utility>
#include <vector>

namespace opendeck::protocol::mdns
{
    /**
     * @brief In-memory mDNS platform hooks used by tests.
     */
    class HwaTest : public Hwa
    {
        public:
        std::array<uint8_t, 12>  serial              = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
        std::string              ip_address_value    = "192.168.1.112";
        std::string              hostname            = {};
        std::string              webconfig_instance  = {};
        std::string              osc_instance        = {};
        IpAddressChangedCallback ip_changed_callback = {};
        bool                     hostname_result     = true;
        bool                     webconfig_result    = true;
        bool                     osc_result          = true;

        /**
         * @brief Copies the configured test serial number.
         *
         * @param buffer Destination buffer.
         *
         * @return Number of bytes copied.
         */
        ssize_t serial_number(std::span<uint8_t> buffer) override
        {
            const auto size = std::min(buffer.size(), serial.size());
            std::copy_n(serial.begin(), size, buffer.begin());
            return static_cast<ssize_t>(size);
        }

        /**
         * @brief Copies the configured test IPv4 address.
         *
         * @param buffer Destination buffer.
         *
         * @return IPv4 address text, or an empty view if it does not fit.
         */
        std::string_view ip_address(std::span<char> buffer) override
        {
            if (ip_address_value.size() >= buffer.size())
            {
                return {};
            }

            std::fill(buffer.begin(), buffer.end(), '\0');
            std::copy(ip_address_value.begin(), ip_address_value.end(), buffer.begin());

            return std::string_view(buffer.data(), ip_address_value.size());
        }

        /**
         * @brief Stores the callback used by tests to emulate IP changes.
         *
         * @param callback Callback to store.
         */
        void register_ip_address_changed_callback(IpAddressChangedCallback callback) override
        {
            ip_changed_callback = std::move(callback);
        }

        /**
         * @brief Stores the hostname requested by the mDNS backend.
         *
         * @param hostname Hostname without the `.local` suffix.
         *
         * @return Configured test result.
         */
        bool set_hostname(std::string_view hostname) override
        {
            this->hostname = std::string(hostname);
            return hostname_result;
        }

        /**
         * @brief Stores the requested DNS-SD WebConfig instance name.
         *
         * @param instance Service instance name.
         *
         * @return Configured test result.
         */
        bool advertise_webconfig(std::string_view instance) override
        {
            webconfig_instance = std::string(instance);
            return webconfig_result;
        }

        /**
         * @brief Stores the requested DNS-SD OSC instance name.
         *
         * @param instance Service instance name.
         *
         * @return Configured test result.
         */
        bool advertise_osc(std::string_view instance) override
        {
            osc_instance = std::string(instance);
            return osc_result;
        }

        /**
         * @brief Triggers the stored IP-change callback.
         */
        void trigger_ip_change()
        {
            if (ip_changed_callback)
            {
                ip_changed_callback();
            }
        }
    };
}    // namespace opendeck::protocol::mdns
