/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "common/src/mdns/instance/impl/mdns.h"

#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <algorithm>
#include <cctype>
#include <utility>

using namespace opendeck::mdns;

namespace
{
    LOG_MODULE_REGISTER(mdns, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

    constexpr char HEX_DIGITS[]           = "0123456789abcdef";
    constexpr int  TARGET_UID_START_SHIFT = 24;
    constexpr auto BYTE_MASK              = 0xFFU;
    constexpr auto HEX_NIBBLE_SHIFT       = 4U;
    constexpr auto HEX_NIBBLE_MASK        = 0x0FU;

    char hostname_safe_char(const char character)
    {
        if (std::isalnum(static_cast<unsigned char>(character)) != 0)
        {
            return static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
        }

        return '-';
    }
}    // namespace

BaseMdns::BaseMdns(Hwa& hwa)
    : _hwa(hwa)
{}

std::string_view BaseMdns::default_hostname(std::string_view serial)
{
    reset();

    for (const auto character : HOSTNAME_PREFIX)
    {
        if (!append_char(character))
        {
            return {};
        }
    }

    if (!append_char('-') || !append_target() || !append_char('-') || !append_serial(serial))
    {
        return {};
    }

    return std::string_view(_buffer.data(), _size);
}

std::string_view BaseMdns::default_hostname()
{
    return default_hostname(_hwa.serial_number());
}

std::string_view BaseMdns::custom_hostname(std::span<const uint8_t> hostname)
{
    reset();

    bool terminated = false;

    for (const auto value : hostname)
    {
        if (value == '\0')
        {
            terminated = true;
            break;
        }

        const auto character = static_cast<char>(value);

        if ((std::isalnum(static_cast<unsigned char>(character)) == 0) && (character != '-'))
        {
            reset();
            return {};
        }

        if (!append_char(static_cast<char>(std::tolower(static_cast<unsigned char>(character)))))
        {
            reset();
            return {};
        }
    }

    if (!terminated || (_size == 0))
    {
        reset();
        return {};
    }

    if ((_buffer.front() == '-') || (_buffer[_size - 1] == '-'))
    {
        reset();
        return {};
    }

    return std::string_view(_buffer.data(), _size);
}

std::string_view BaseMdns::network_name()
{
    if (_size + LOCAL_SUFFIX.size() >= _buffer.size())
    {
        return {};
    }

    std::copy(LOCAL_SUFFIX.begin(), LOCAL_SUFFIX.end(), _buffer.begin() + _size);
    _buffer[_size + LOCAL_SUFFIX.size()] = '\0';

    return std::string_view(_buffer.data(), _size + LOCAL_SUFFIX.size());
}

std::string_view BaseMdns::ip_address(std::span<char> buffer)
{
    return _hwa.ip_address(buffer);
}

NetworkIdentity BaseMdns::network_identity(std::span<char> ip_address_buffer)
{
    return {
        .name       = network_name(),
        .ip_address = ip_address(ip_address_buffer),
    };
}

void BaseMdns::log_network_identity(const NetworkIdentity& identity)
{
    if (identity.ip_address.empty())
    {
        LOG_INF("mDNS network identity: %.*s",
                static_cast<int>(identity.name.size()),
                identity.name.data());
        return;
    }

    LOG_INF("mDNS network identity: %.*s %.*s",
            static_cast<int>(identity.name.size()),
            identity.name.data(),
            static_cast<int>(identity.ip_address.size()),
            identity.ip_address.data());
}

void BaseMdns::register_ip_address_changed_callback(Hwa::IpAddressChangedCallback callback)
{
    _hwa.register_ip_address_changed_callback(std::move(callback));
}

bool BaseMdns::set_hostname(std::string_view hostname)
{
    return _hwa.set_hostname(hostname);
}

bool BaseMdns::advertise_service(std::string_view instance,
                                 std::span<char>  buffer,
                                 uint16_t&        service_port,
                                 const uint16_t   port)
{
    return _hwa.advertise_service(instance, buffer, service_port, port);
}

bool BaseMdns::advertise_service(Service service, std::string_view instance)
{
    return advertise_service(instance, service.instance, service.port, service.service_port);
}

bool BaseMdns::append_target()
{
    for (const auto* character = OPENDECK_TARGET; *character != '\0'; character++)
    {
        if (!append_char(hostname_safe_char(*character)))
        {
            return false;
        }
    }

    return true;
}

bool BaseMdns::append_serial(std::string_view serial)
{
    if (!serial.empty())
    {
        for (const auto character : serial)
        {
            if (!append_char(character))
            {
                return false;
            }
        }

        return true;
    }

    return append_target_uid();
}

bool BaseMdns::append_target_uid()
{
    for (int shift = TARGET_UID_START_SHIFT; shift >= 0; shift -= BITS_PER_BYTE)
    {
        if (!append_hex_byte(static_cast<uint8_t>((OPENDECK_TARGET_UID >> shift) & BYTE_MASK)))
        {
            return false;
        }
    }

    return true;
}

bool BaseMdns::append_hex_byte(const uint8_t byte)
{
    return append_char(HEX_DIGITS[(byte >> HEX_NIBBLE_SHIFT) & HEX_NIBBLE_MASK]) &&
           append_char(HEX_DIGITS[byte & HEX_NIBBLE_MASK]);
}

bool BaseMdns::append_char(const char character)
{
    if (_size + 1 >= _buffer.size())
    {
        return false;
    }

    _buffer[_size++] = character;
    _buffer[_size]   = '\0';

    return true;
}

void BaseMdns::reset()
{
    std::fill(_buffer.begin(), _buffer.end(), '\0');
    _size = 0;
}
