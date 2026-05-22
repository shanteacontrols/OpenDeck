/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/protocol/mdns/instance/impl/mdns.h"
#include "firmware/src/protocol/mdns/shared/common.h"
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/util/configurable/configurable.h"

#include <zephyr/logging/log.h>

#include <array>

using namespace opendeck::protocol::mdns;

namespace
{
    LOG_MODULE_REGISTER(mdns, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

}    // namespace

Mdns::Mdns(opendeck::mdns::BaseMdns& base_mdns, Services& services, Database& database)
    : _base_mdns(base_mdns)
    , _services(services)
    , _database(database)
    , _network_identity_work([this]()
                             {
                                 const auto identity = _base_mdns.network_identity(_ip_address);

                                 if (identity.name.empty())
                                 {
                                     return;
                                 }

                                 publish_network_identity(identity);
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

    const auto identity = _base_mdns.network_identity(_ip_address);

    if (identity.name.empty())
    {
        LOG_WRN("Failed to build mDNS network name");
        return false;
    }

    if (!_base_mdns.set_hostname(hostname))
    {
        LOG_WRN("Failed to set mDNS hostname");
        return false;
    }

    if (!_base_mdns.advertise_service(_services.webconfig(), hostname))
    {
        LOG_WRN("Failed to advertise WebConfig over mDNS");
        return false;
    }

    if (!_base_mdns.advertise_service(_services.osc(), hostname))
    {
        LOG_WRN("Failed to advertise OSC over mDNS");
        return false;
    }

    _base_mdns.register_ip_address_changed_callback(
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
    _base_mdns.register_ip_address_changed_callback({});
    return true;
}

std::string_view Mdns::make_hostname()
{
    const auto custom_hostname = append_custom_hostname();

    if (!custom_hostname.empty())
    {
        return custom_hostname;
    }

    return _base_mdns.default_hostname();
}

std::string_view Mdns::append_custom_hostname()
{
    load_custom_hostname();

    const auto hostname = _base_mdns.custom_hostname(_custom_hostname);

    if (!hostname.empty())
    {
        return hostname;
    }

    if (_custom_hostname.front() != '\0')
    {
        LOG_WRN("Ignoring invalid custom mDNS hostname");
    }

    return {};
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

void Mdns::publish_network_identity(const opendeck::mdns::NetworkIdentity& identity)
{
    opendeck::mdns::BaseMdns::log_network_identity(identity);

    signaling::publish(signaling::NetworkIdentitySignal(identity.name, identity.ip_address));
}

void Mdns::schedule_network_identity_publish()
{
    _network_identity_work.reschedule(0);
}

void Mdns::handle_ip_address_changed()
{
    schedule_network_identity_publish();
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
