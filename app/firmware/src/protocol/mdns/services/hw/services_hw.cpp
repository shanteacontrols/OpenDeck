/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/protocol/mdns/services/hw/services_hw.h"

using namespace opendeck::protocol::mdns;

void ServicesHw::register_service(ServiceProvider* provider)
{
    _providers.push_back(provider);
}

std::span<ServiceProvider* const> ServicesHw::services()
{
    return std::span<ServiceProvider* const>(_providers.data(), _providers.size());
}
