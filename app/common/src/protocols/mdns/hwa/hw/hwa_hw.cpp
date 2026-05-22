/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "common/src/protocols/mdns/hwa/hw/hwa_hw.h"
#include "common/src/protocols/mdns/shared/common.h"

#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/hostname.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/sys/byteorder.h>

#include <array>
#include <algorithm>
#include <utility>

using namespace opendeck::common::protocols::mdns;

LOG_MODULE_REGISTER(opendeck_mdns_hwa, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

namespace
{
    constexpr char HEX_DIGITS[]     = "0123456789abcdef";
    constexpr auto HEX_NIBBLE_SHIFT = 4U;
    constexpr auto HEX_NIBBLE_MASK  = 0x0FU;

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

HwaHw::HwaHw(opendeck::common::mcu::Hwa& mcu)
    : _mcu(mcu)
{}

std::string_view HwaHw::serial_number()
{
    if (_serial_number_loaded)
    {
        return std::string_view(_serial_number.data());
    }

    const auto serial = _mcu.serial_number();

    if (serial.empty())
    {
        _serial_number_loaded = true;
        return {};
    }

    const auto start = serial.size() > SERIAL_SUFFIX_BYTES ? serial.size() - SERIAL_SUFFIX_BYTES : 0;
    size_t     index = 0;

    for (size_t i = start; i < serial.size(); i++)
    {
        _serial_number[index++] = HEX_DIGITS[(serial[i] >> HEX_NIBBLE_SHIFT) & HEX_NIBBLE_MASK];
        _serial_number[index++] = HEX_DIGITS[serial[i] & HEX_NIBBLE_MASK];
    }

    _serial_number_loaded = true;

    return std::string_view(_serial_number.data(), index);
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

bool HwaHw::advertise_service(std::string_view instance,
                              std::span<char>  buffer,
                              uint16_t&        service_port,
                              const uint16_t   port)
{
    if (instance.size() >= buffer.size())
    {
        return false;
    }

    std::fill(buffer.begin(), buffer.end(), '\0');
    std::copy(instance.begin(), instance.end(), buffer.begin());
    service_port = sys_cpu_to_be16(port);

    return true;
}

void HwaHw::ip_event_handler(net_mgmt_event_callback* callback,
                             uint64_t                 event,
                             [[maybe_unused]] net_if* iface)
{
    auto self = IpEventCallback::extract_user_data(static_cast<void*>(callback));

    if (self == nullptr)
    {
        return;
    }

    if (event == NET_EVENT_IPV4_ADDR_ADD)
    {
        std::array<char, IPV4_ADDRESS_SIZE> buffer  = {};
        const auto                          address = self->ip_address(buffer);

        if (!address.empty())
        {
            LOG_INF("IPv4 address assigned: %.*s", static_cast<int>(address.size()), address.data());
        }
    }
    else if (event == NET_EVENT_IPV4_ADDR_DEL)
    {
        LOG_INF("IPv4 address removed");
    }

    if (self->_ip_address_changed_callback)
    {
        self->_ip_address_changed_callback();
    }
}
