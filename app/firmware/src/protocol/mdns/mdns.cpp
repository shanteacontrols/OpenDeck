/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/protocol/mdns/mdns.h"
#include "firmware/src/protocol/mdns/common.h"
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/util/configurable/configurable.h"

#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <algorithm>
#include <cctype>

using namespace opendeck::protocol::mdns;

namespace
{
    LOG_MODULE_REGISTER(mdns, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

    constexpr char HEX_DIGITS[]           = "0123456789abcdef";
    constexpr int  TARGET_UID_START_SHIFT = 24;
    constexpr auto BYTE_MASK              = 0xFFU;
    constexpr auto HEX_NIBBLE_SHIFT       = 4U;
    constexpr auto HEX_NIBBLE_MASK        = 0x0FU;

    /**
     * @brief Converts one character to a hostname-safe lowercase character.
     *
     * @param character Character from the OpenDeck target name.
     *
     * @return Lowercase alphanumeric character, or `-`.
     */
    char hostname_safe_char(char character)
    {
        if (std::isalnum(static_cast<unsigned char>(character)) != 0)
        {
            return static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
        }

        return '-';
    }
}    // namespace

Mdns::Mdns(Hwa& hwa, Database& database)
    : _hwa(hwa)
    , _database(database)
    , _network_identity_work([this]()
                             {
                                 const auto name = make_network_name();

                                 if (name.empty())
                                 {
                                     return;
                                 }

                                 publish_network_identity(name);
                             },
                             []()
                             {
                                 return threads::SystemWorkqueue::handle();
                             })
{
    ConfigHandler.register_config(
        // read
        sys::Config::Block::Global,
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sys_config_get(static_cast<sys::Config::Section::Global>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sys_config_set(static_cast<sys::Config::Section::Global>(section), index, value);
        });
}

bool Mdns::init()
{
    const auto hostname = make_hostname();

    if (hostname.empty())
    {
        LOG_WRN("Failed to build mDNS hostname");
        return false;
    }

    const auto name = make_network_name();

    if (name.empty())
    {
        LOG_WRN("Failed to build mDNS network name");
        return false;
    }

    if (!_hwa.set_hostname(hostname))
    {
        LOG_WRN("Failed to set mDNS hostname");
        return false;
    }

    if (!_hwa.advertise_webconfig(hostname))
    {
        LOG_WRN("Failed to advertise WebConfig over mDNS");
        return false;
    }

    if (!_hwa.advertise_osc(hostname))
    {
        LOG_WRN("Failed to advertise OSC over mDNS");
        return false;
    }

    _hwa.register_ip_address_changed_callback(
        [this]()
        {
            handle_ip_address_changed();
        });

    schedule_network_identity_publish();

    return true;
}

bool Mdns::deinit()
{
    _network_identity_work.cancel();
    _hwa.register_ip_address_changed_callback({});
    return true;
}

std::string_view Mdns::make_hostname()
{
    reset_hostname();

    if (append_custom_hostname())
    {
        return std::string_view(_hostname.data(), _hostname_size);
    }

    for (const auto character : HOSTNAME_PREFIX)
    {
        if (!append_char(character))
        {
            return {};
        }
    }

    if (!append_char('-') || !append_target() || !append_char('-') || !append_serial())
    {
        return {};
    }

    return std::string_view(_hostname.data(), _hostname_size);
}

bool Mdns::append_custom_hostname()
{
    load_custom_hostname();
    reset_hostname();

    bool terminated = false;

    for (size_t i = 0; i < CUSTOM_HOSTNAME_DB_SIZE; i++)
    {
        const auto value = _custom_hostname[i];

        if (value == '\0')
        {
            terminated = true;
            break;
        }

        const auto character = static_cast<char>(value);

        if ((std::isalnum(static_cast<unsigned char>(character)) == 0) && (character != '-'))
        {
            LOG_WRN("Ignoring invalid custom mDNS hostname");
            reset_hostname();
            return false;
        }

        if (!append_char(static_cast<char>(std::tolower(static_cast<unsigned char>(character)))))
        {
            reset_hostname();
            return false;
        }
    }

    if (!terminated)
    {
        LOG_WRN("Ignoring unterminated custom mDNS hostname");
        reset_hostname();
        return false;
    }

    if (_hostname_size == 0)
    {
        return false;
    }

    if ((_hostname.front() == '-') || (_hostname[_hostname_size - 1] == '-'))
    {
        LOG_WRN("Ignoring invalid custom mDNS hostname");
        reset_hostname();
        return false;
    }

    return true;
}

void Mdns::load_custom_hostname()
{
    if (_custom_hostname_loaded)
    {
        return;
    }

    _custom_hostname.fill(0);

    for (size_t i = 0; i < _custom_hostname.size(); i++)
    {
        const auto value = static_cast<uint8_t>(_database.read(database::Config::Section::Common::MdnsHostname, i));

        _custom_hostname[i] = value;

        if (value == '\0')
        {
            break;
        }
    }

    _custom_hostname_loaded = true;
}

std::string_view Mdns::make_network_name()
{
    if (_hostname_size + LOCAL_SUFFIX.size() >= _hostname.size())
    {
        return {};
    }

    std::copy(LOCAL_SUFFIX.begin(), LOCAL_SUFFIX.end(), _hostname.begin() + _hostname_size);
    _hostname[_hostname_size + LOCAL_SUFFIX.size()] = '\0';

    return std::string_view(_hostname.data(), _hostname_size + LOCAL_SUFFIX.size());
}

void Mdns::publish_network_identity(std::string_view name)
{
    const auto ip_address = _hwa.ip_address(_ip_address);

    if (ip_address.empty())
    {
        LOG_INF("mDNS network identity: %.*s",
                static_cast<int>(name.size()),
                name.data());
    }
    else
    {
        LOG_INF("mDNS network identity: %.*s %.*s",
                static_cast<int>(name.size()),
                name.data(),
                static_cast<int>(ip_address.size()),
                ip_address.data());
    }

    signaling::publish(signaling::NetworkIdentitySignal(name, ip_address));
}

void Mdns::schedule_network_identity_publish()
{
    _network_identity_work.reschedule(0);
}

void Mdns::handle_ip_address_changed()
{
    schedule_network_identity_publish();
}

bool Mdns::append_target()
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

bool Mdns::append_serial()
{
    std::array<uint8_t, SERIAL_BUFFER_SIZE> serial = {};
    const auto                              size   = _hwa.serial_number(serial);

    if (size > 0)
    {
        const auto copied = static_cast<size_t>(size);
        const auto start  = copied > SERIAL_SUFFIX_BYTES ? copied - SERIAL_SUFFIX_BYTES : 0;

        for (size_t i = start; i < copied; i++)
        {
            if (!append_hex_byte(serial[i]))
            {
                return false;
            }
        }

        return true;
    }

    for (int shift = TARGET_UID_START_SHIFT; shift >= 0; shift -= BITS_PER_BYTE)
    {
        if (!append_hex_byte(static_cast<uint8_t>((OPENDECK_TARGET_UID >> shift) & BYTE_MASK)))
        {
            return false;
        }
    }

    return true;
}

bool Mdns::append_hex_byte(uint8_t byte)
{
    return append_char(HEX_DIGITS[(byte >> HEX_NIBBLE_SHIFT) & HEX_NIBBLE_MASK]) &&
           append_char(HEX_DIGITS[byte & HEX_NIBBLE_MASK]);
}

bool Mdns::append_char(char character)
{
    if (_hostname_size + 1 >= _hostname.size())
    {
        return false;
    }

    _hostname[_hostname_size++] = character;
    _hostname[_hostname_size]   = '\0';

    return true;
}

void Mdns::reset_hostname()
{
    _hostname.fill('\0');
    _hostname_size = 0;
}

std::optional<uint8_t> Mdns::sys_config_get(sys::Config::Section::Global section, size_t index, uint16_t& value)
{
    if (section != sys::Config::Section::Global::MdnsHostname)
    {
        return {};
    }

    value = _custom_hostname[index];
    return sys::Config::Status::Ack;
}

std::optional<uint8_t> Mdns::sys_config_set(sys::Config::Section::Global section, size_t index, uint16_t value)
{
    if (section != sys::Config::Section::Global::MdnsHostname)
    {
        return {};
    }

    auto result = _database.update(database::Config::Section::Common::MdnsHostname, index, value)
                      ? sys::Config::Status::Ack
                      : sys::Config::Status::ErrorWrite;

    if (result == sys::Config::Status::Ack)
    {
        _custom_hostname[index] = static_cast<uint8_t>(value);
        _custom_hostname_loaded = true;
    }

    return result;
}
