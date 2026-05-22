/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/protocols/mdns/instance/impl/mdns.h"

#ifdef CONFIG_PROJECT_BOOTLOADER_SUPPORT_MDNS

#include "bootloader/src/protocols/mdns/shared/common.h"
#include "common/src/protocols/websockets/shared/common.h"

#include <zephyr/logging/log.h>

using namespace opendeck::bootloader::protocols::mdns;

namespace
{
    LOG_MODULE_REGISTER(opendeck_bootloader_mdns, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}

Mdns::Mdns(opendeck::common::protocols::mdns::BaseMdns& base_mdns, Services& services)
    : _base_mdns(base_mdns)
    , _services(services)
{}

bool Mdns::init()
{
    const auto hostname = _base_mdns.default_hostname();

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

    if (!_base_mdns.advertise_service(_services.dfu(), hostname))
    {
        LOG_WRN("Failed to advertise DFU over mDNS");
        return false;
    }

    LOG_INF("mDNS DFU service: %.*s.%s %.*s.%s port %u",
            static_cast<int>(hostname.size()),
            hostname.data(),
            opendeck::common::protocols::mdns::LOCAL_DOMAIN.data(),
            static_cast<int>(DFU_SERVICE.size()),
            DFU_SERVICE.data(),
            opendeck::common::protocols::mdns::TCP_PROTOCOL.data(),
            opendeck::common::protocols::websockets::DEFAULT_PORT);

    _base_mdns.register_ip_address_changed_callback([this]
                                                    {
                                                        handle_ip_address_changed();
                                                    });
    publish_network_identity(identity);

    return true;
}

void Mdns::publish_network_identity(const opendeck::common::protocols::mdns::NetworkIdentity& identity)
{
    opendeck::common::protocols::mdns::BaseMdns::log_network_identity(identity);
}

void Mdns::handle_ip_address_changed()
{
    const auto identity = _base_mdns.network_identity(_ip_address);

    if (identity.name.empty())
    {
        return;
    }

    publish_network_identity(identity);
}

#endif
