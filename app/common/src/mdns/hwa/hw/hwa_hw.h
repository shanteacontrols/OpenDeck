/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/mcu/shared/deps.h"
#include "common/src/mdns/shared/deps.h"

#include "zlibs/utils/misc/userdata_struct.h"

#include <zephyr/net/net_mgmt.h>

#include <array>
#include <span>

namespace opendeck::mdns
{
    /**
     * @brief Zephyr-backed mDNS, DNS-SD, and IPv4 event hooks.
     */
    class HwaHw : public virtual Hwa
    {
        public:
        /**
         * @brief Constructs Zephyr-backed mDNS hooks.
         *
         * @param mcu MCU services used for hardware identity.
         */
        explicit HwaHw(mcu::Hwa& mcu);

        /**
         * @brief Returns the MCU serial suffix used in the default hostname.
         *
         * @return Lowercase hexadecimal serial suffix, or an empty view when unavailable.
         */
        std::string_view serial_number() override;

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
         * @brief Updates a static DNS-SD service record instance and port.
         *
         * @param instance Service instance name shown by discovery tools.
         * @param buffer DNS-SD record instance storage.
         * @param service_port DNS-SD record port storage.
         * @param port Service port in host byte order.
         *
         * @return `true` if the instance fits in `buffer`.
         */
        bool advertise_service(std::string_view instance,
                               std::span<char>  buffer,
                               uint16_t&        service_port,
                               uint16_t         port) override;

        private:
        static constexpr size_t SERIAL_SUFFIX_BYTES = 4;

        using IpEventCallback = zlibs::utils::misc::UserDataStruct<net_mgmt_event_callback, HwaHw>;

        mcu::Hwa&                                         _mcu;
        std::array<char, (SERIAL_SUFFIX_BYTES * 2U) + 1U> _serial_number                = {};
        bool                                              _serial_number_loaded         = false;
        IpAddressChangedCallback                          _ip_address_changed_callback  = {};
        IpEventCallback                                   _ip_event_callback            = {};
        bool                                              _ip_event_callback_registered = false;

        /**
         * @brief Dispatches Zephyr IPv4 address events into the stored callback.
         *
         * @param callback Registered Zephyr callback object.
         * @param event Network event ID.
         * @param iface Interface that emitted the event.
         */
        static void ip_event_handler(net_mgmt_event_callback* callback, uint64_t event, net_if* iface);
    };
}    // namespace opendeck::mdns
