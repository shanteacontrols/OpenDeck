/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/protocol/mdns/hwa/hw/hwa_hw.h"

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_MDNS

#include "firmware/src/protocol/mdns/shared/common.h"
#include "firmware/src/protocol/osc/shared/common.h"
#include "firmware/src/protocol/webconfig/shared/common.h"

#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/dns_sd.h>
#include <zephyr/net/hostname.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/sys/byteorder.h>

#include <algorithm>
#include <cstring>
#include <utility>

using namespace opendeck::protocol::mdns;

LOG_MODULE_REGISTER(network_init, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

namespace
{
    char     webconfig_instance[NET_HOSTNAME_SIZE] = CONFIG_NET_HOSTNAME;
    uint16_t webconfig_port                        = sys_cpu_to_be16(opendeck::protocol::webconfig::DEFAULT_PORT);
    char     osc_instance[NET_HOSTNAME_SIZE]       = CONFIG_NET_HOSTNAME;
    uint16_t osc_port                              = sys_cpu_to_be16(opendeck::protocol::osc::DEFAULT_LISTEN_PORT);

    DNS_SD_REGISTER_SERVICE(opendeck_webconfig,
                            webconfig_instance,
                            WEBCONFIG_SERVICE.data(),
                            WEBCONFIG_PROTOCOL.data(),
                            LOCAL_DOMAIN.data(),
                            WEBCONFIG_TXT,
                            &webconfig_port);

    DNS_SD_REGISTER_SERVICE(opendeck_osc,
                            osc_instance,
                            OSC_SERVICE.data(),
                            OSC_PROTOCOL.data(),
                            LOCAL_DOMAIN.data(),
                            DNS_SD_EMPTY_TXT,
                            &osc_port);

    /**
     * @brief Copies text into a fixed buffer and leaves it null-terminated.
     *
     * @param value Text to copy.
     * @param buffer Destination buffer.
     *
     * @return `true` if the text fits with a trailing null byte.
     */
    bool copy_to_buffer(std::string_view value, std::span<char> buffer)
    {
        if (value.size() >= buffer.size())
        {
            return false;
        }

        std::fill(buffer.begin(), buffer.end(), '\0');
        std::copy(value.begin(), value.end(), buffer.begin());

        return true;
    }

#ifdef CONFIG_NET_CONFIG_AUTO_INIT
    int log_network_auto_init_wait()
    {
        if constexpr (CONFIG_NET_CONFIG_INIT_TIMEOUT == 0)
        {
            LOG_INF("Network auto-init starting without waiting for link or IPv4 address");
        }
        else
        {
            LOG_INF("Network auto-init waiting up to %d seconds for link and IPv4 address",
                    CONFIG_NET_CONFIG_INIT_TIMEOUT);
        }

        return 0;
    }

    SYS_INIT(log_network_auto_init_wait, APPLICATION, 0);
#endif
}    // namespace

HwaHw::HwaHw(mcu::Hwa& mcu)
    : _mcu(mcu)
{}

ssize_t HwaHw::serial_number(std::span<uint8_t> buffer)
{
    const auto serial = _mcu.serial_number();
    const auto size   = std::min(buffer.size(), serial.size());

    std::copy_n(serial.begin(), size, buffer.begin());

    return static_cast<ssize_t>(size);
}

std::string_view HwaHw::ip_address(std::span<char> buffer)
{
    const auto iface = net_if_get_default();

    if ((iface == nullptr) || (iface->config.ip.ipv4 == nullptr) || buffer.empty())
    {
        return {};
    }

    for (size_t i = 0; i < NET_IF_MAX_IPV4_ADDR; i++)
    {
        const auto& address = iface->config.ip.ipv4->unicast[i].ipv4;

        if (!address.is_used || net_ipv4_is_addr_unspecified(&address.address.in_addr))
        {
            continue;
        }

        const auto result = net_addr_ntop(AF_INET, &address.address.in_addr, buffer.data(), buffer.size());

        if (result == nullptr)
        {
            return {};
        }

        return std::string_view(buffer.data());
    }

    return {};
}

void HwaHw::register_ip_address_changed_callback(IpAddressChangedCallback callback)
{
    _ip_address_changed_callback = std::move(callback);

    if (!_ip_address_changed_callback)
    {
        if (_ip_event_callback_registered)
        {
            net_mgmt_del_event_callback(&_ip_event_callback.member);
            _ip_event_callback_registered = false;
        }

        _ip_event_callback.user_data = nullptr;
        return;
    }

    _ip_event_callback.user_data = this;

    if (_ip_event_callback_registered)
    {
        return;
    }

    net_mgmt_init_event_callback(&_ip_event_callback.member,
                                 ip_event_handler,
                                 NET_EVENT_IPV4_ADDR_ADD | NET_EVENT_IPV4_ADDR_DEL);    // NOLINT(misc-redundant-expression): Zephyr event masks are intentionally ORed.
    net_mgmt_add_event_callback(&_ip_event_callback.member);
    _ip_event_callback_registered = true;
}

bool HwaHw::set_hostname(std::string_view hostname)
{
    return net_hostname_set(hostname.data(), hostname.size()) == 0;
}

bool HwaHw::advertise_webconfig(std::string_view instance)
{
    return copy_to_buffer(instance, webconfig_instance);
}

bool HwaHw::advertise_osc(std::string_view instance)
{
    return copy_to_buffer(instance, osc_instance);
}

void HwaHw::ip_event_handler(net_mgmt_event_callback*  callback,
                             [[maybe_unused]] uint64_t event,
                             [[maybe_unused]] net_if*  iface)
{
    auto self = IpEventCallback::extract_user_data(static_cast<void*>(callback));

    if (self == nullptr)
    {
        return;
    }

    if (self->_ip_address_changed_callback)
    {
        self->_ip_address_changed_callback();
    }
}

#endif
