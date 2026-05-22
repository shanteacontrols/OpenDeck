/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/mdns/instance/impl/mdns.h"

#ifdef CONFIG_PROJECT_BOOTLOADER_SUPPORT_MDNS

using namespace opendeck::bootloader::mdns;

Mdns::Mdns(opendeck::mdns::BaseMdns& base_mdns, Services& services)
    : _base_mdns(base_mdns)
    , _services(services)
{}

bool Mdns::init()
{
    const auto hostname = _base_mdns.default_hostname();

    if (hostname.empty())
    {
        return false;
    }

    return _base_mdns.set_hostname(hostname) &&
           _base_mdns.advertise_service(_services.recovery(), hostname);
}

#endif
