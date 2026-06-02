/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/mdns/shared/deps.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace opendeck::firmware::protocol::mdns
{
    /**
     * @brief In-memory mDNS platform hooks used by tests.
     */
    class HwaTest : public Hwa
    {
        public:
        std::string                   serial              = "08090a0b";
        std::string                   ip_address_value    = "192.168.1.112";
        std::string                   hostname            = {};
        std::string                   service_instance    = {};
        Hwa::IpAddressChangedCallback ip_changed_callback = {};
        bool                          hostname_result     = true;
        bool                          service_result      = true;
        size_t                        advertise_count     = 0;
        size_t                        advertise_fail_at   = 0;

        /**
         * @brief Returns the configured test serial number suffix.
         *
         * @return Serial suffix text.
         */
        std::string_view serial_number() override
        {
            return serial;
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
        void register_ip_address_changed_callback(Hwa::IpAddressChangedCallback callback) override
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
         * @brief Stores the requested generic DNS-SD service instance name.
         *
         * @param instance Service instance name.
         * @param buffer DNS-SD record instance storage.
         * @param service_port DNS-SD record port storage.
         * @param port Service port in host byte order.
         *
         * @return Configured test result.
         */
        bool advertise_service(std::string_view instance,
                               std::span<char>  buffer,
                               uint16_t&        service_port,
                               const uint16_t   port) override
        {
            advertise_count++;

            if ((advertise_fail_at != 0U) && (advertise_count == advertise_fail_at))
            {
                return false;
            }

            service_instance = std::string(instance);

            if (instance.size() >= buffer.size())
            {
                return false;
            }

            std::fill(buffer.begin(), buffer.end(), '\0');
            std::copy(instance.begin(), instance.end(), buffer.begin());
            service_port = port;

            return service_result;
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
}    // namespace opendeck::firmware::protocol::mdns
