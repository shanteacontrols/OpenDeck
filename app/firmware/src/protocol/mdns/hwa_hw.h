/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

#include "zlibs/utils/misc/userdata_struct.h"

#include <zephyr/net/net_mgmt.h>

namespace opendeck::protocol::mdns
{
    /**
     * @brief Zephyr-backed mDNS platform hooks.
     */
    class HwaHw : public Hwa
    {
        public:
        /**
         * @brief Reads Zephyr's hardware ID bytes.
         *
         * @param buffer Destination buffer.
         *
         * @return Number of bytes copied, otherwise a negative errno value.
         */
        ssize_t serial_number(std::span<uint8_t> buffer) override;

        /**
         * @brief Copies the IPv4 address assigned to Zephyr's default interface.
         *
         * @param buffer Destination buffer.
         *
         * @return IPv4 address text, or an empty view if unavailable.
         */
        std::string_view ip_address(std::span<char> buffer) override;

        /**
         * @brief Registers for Zephyr IPv4 address add/remove events.
         *
         * @param callback Callback to call after an IPv4 address event.
         */
        void register_ip_address_changed_callback(IpAddressChangedCallback callback) override;

        /**
         * @brief Sets Zephyr's runtime network hostname.
         *
         * @param hostname Hostname without the `.local` suffix.
         *
         * @return `true` if Zephyr accepted the hostname.
         */
        bool set_hostname(std::string_view hostname) override;

        /**
         * @brief Updates the DNS-SD WebConfig instance name.
         *
         * @param instance Service instance name shown by discovery tools.
         *
         * @return `true` if the name fits in Zephyr's hostname buffer.
         */
        bool advertise_webconfig(std::string_view instance) override;

        /**
         * @brief Updates the DNS-SD OSC instance name.
         *
         * @param instance Service instance name shown by discovery tools.
         *
         * @return `true` if the name fits in Zephyr's hostname buffer.
         */
        bool advertise_osc(std::string_view instance) override;

        private:
        using IpEventCallback = zlibs::utils::misc::UserDataStruct<net_mgmt_event_callback, HwaHw>;

        IpAddressChangedCallback _ip_address_changed_callback  = {};
        IpEventCallback          _ip_event_callback            = {};
        bool                     _ip_event_callback_registered = false;

        /**
         * @brief Dispatches Zephyr IPv4 address events into the stored callback.
         *
         * @param callback Registered Zephyr callback object.
         * @param event Network event ID.
         * @param iface Interface that emitted the event.
         */
        static void ip_event_handler(net_mgmt_event_callback* callback, uint64_t event, net_if* iface);
    };
}    // namespace opendeck::protocol::mdns
