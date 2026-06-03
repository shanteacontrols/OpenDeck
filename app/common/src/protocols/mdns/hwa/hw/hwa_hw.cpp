/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "common/src/protocols/mdns/hwa/hw/hwa_hw.h"
#include "common/src/mcu/shared/common.h"
#include "common/src/protocols/mdns/shared/common.h"
#include "zlibs/utils/misc/bit.h"

#include <zephyr/device.h>
#include <zephyr/drivers/hwinfo.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/ethernet.h>
#include <zephyr/net/hostname.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/crc.h>
#include <zephyr/sys/util_macro.h>

#include <array>
#include <algorithm>
#include <cstring>
#include <optional>
#include <utility>

using namespace opendeck::common::protocols::mdns;

LOG_MODULE_REGISTER(opendeck_mdns_hwa, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

namespace
{
    constexpr char HEX_DIGITS[]     = "0123456789abcdef";
    constexpr auto HEX_NIBBLE_SHIFT = 4U;
    constexpr auto HEX_NIBBLE_MASK  = 0x0FU;

#ifdef CONFIG_NET_CONFIG_AUTO_INIT
#define OPENDECK_STABLE_MAC_INIT_PRIO UTIL_DEC(CONFIG_NET_INIT_PRIO)

    using MacAddress = std::array<uint8_t, NET_ETH_ADDR_LEN>;

    constexpr auto STABLE_MAC_INIT_PRIO = OPENDECK_STABLE_MAC_INIT_PRIO;
    constexpr auto SERIAL_CRC_MIX_MASK  = 0xA5U;
    constexpr auto MAC_ADDRESS_BYTE_5   = 5U;
    constexpr auto MAC_UNICAST_MASK     = 0xFEU;

    constexpr uint8_t crc_byte(uint32_t crc, size_t byte)
    {
        return static_cast<uint8_t>((crc >> (byte * zlibs::utils::misc::BYTE_BIT_COUNT)) & zlibs::utils::misc::BYTE_MASK);
    }

    std::optional<MacAddress> make_stable_mac()
    {
        std::array<uint8_t, opendeck::common::mcu::SERIAL_NUMBER_BUFFER_SIZE> serial = {};

        const auto size = hwinfo_get_device_id(serial.data(), serial.size());

        if (size <= 0)
        {
            LOG_WRN("Stable Ethernet MAC unavailable: MCU serial number unavailable");
            return std::nullopt;
        }

        const auto serial_size = static_cast<size_t>(size) > serial.size()
                                     ? serial.size()
                                     : static_cast<size_t>(size);
        const auto lower       = crc32_ieee(serial.data(), serial_size);
        serial[0]              = static_cast<uint8_t>(serial[0] ^ SERIAL_CRC_MIX_MASK);
        const auto upper       = crc32_ieee(serial.data(), serial_size);
        MacAddress mac         = {};

        mac[0]                  = crc_byte(lower, 0U);
        mac[1]                  = crc_byte(lower, 1U);
        mac[2]                  = crc_byte(lower, 2U);
        mac[3]                  = crc_byte(lower, 3U);
        mac[4]                  = crc_byte(upper, 0U);
        mac[MAC_ADDRESS_BYTE_5] = crc_byte(upper, 1U);

        /* Locally administered, unicast MAC derived from the hardware serial. */
        mac[0] = static_cast<uint8_t>((mac[0] | 0x02U) & MAC_UNICAST_MASK);

        return mac;
    }

    int set_stable_ethernet_mac()
    {
        static_assert(CONFIG_ETH_INIT_PRIORITY < STABLE_MAC_INIT_PRIO,
                      "Stable Ethernet MAC must run after Ethernet device init");
        static_assert(STABLE_MAC_INIT_PRIO < CONFIG_NET_INIT_PRIO,
                      "Stable Ethernet MAC must run before network interface init");

        const auto iface = net_if_get_default();

        if (iface == nullptr)
        {
            LOG_WRN("Stable Ethernet MAC unavailable: no default network interface");
            return 0;
        }

        const auto mac = make_stable_mac();

        if (!mac)
        {
            return 0;
        }

        const auto device = net_if_get_device(iface);

        if ((device == nullptr) || (device->api == nullptr))
        {
            LOG_WRN("Stable Ethernet MAC unavailable: no Ethernet device");
            return 0;
        }

        const auto api = static_cast<const ethernet_api*>(device->api);

        if (api->set_config == nullptr)
        {
            LOG_WRN("Stable Ethernet MAC unavailable: Ethernet driver does not support MAC updates");
            return 0;
        }

        ethernet_config config = {};
        std::memcpy(config.mac_address.addr, mac->data(), mac->size());

        const auto result = api->set_config(device,
                                            iface,
                                            ETHERNET_CONFIG_TYPE_MAC_ADDRESS,
                                            &config);

        if (result != 0)
        {
            LOG_WRN("Stable Ethernet MAC rejected: %d", result);
            return 0;
        }

        LOG_INF("Stable Ethernet MAC: %02x:%02x:%02x:%02x:%02x:%02x",
                (*mac)[0],
                (*mac)[1],
                (*mac)[2],
                (*mac)[3],
                (*mac)[4],
                (*mac)[5]);

        return 0;
    }

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

    SYS_INIT(set_stable_ethernet_mac, POST_KERNEL, OPENDECK_STABLE_MAC_INIT_PRIO);
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
