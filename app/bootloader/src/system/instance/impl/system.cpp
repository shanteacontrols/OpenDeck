/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/system/instance/impl/system.h"

#include <zephyr/logging/log.h>

using namespace opendeck::bootloader;

namespace
{
    LOG_MODULE_REGISTER(opendeck_bootloader_system, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

system::System::System(Hwa& hwa)
    : _hwa(hwa)
{}

bool system::System::init()
{
    LOG_INF("OpenDeck bootloader starting");

    _hwa.init_indicators();

    if (_hwa.consume_staged_update())
    {
        LOG_INF("Staged firmware update consumed, rebooting to application");
        _hwa.reboot_application();
    }

    if (!_hwa.init_webusb())
    {
        LOG_ERR("Failed to initialize WebUSB DFU transport");
        return false;
    }

    if (!_hwa.init_websockets())
    {
        LOG_ERR("Failed to initialize WebSocket DFU transport");
        return false;
    }

    if (!_hwa.init_mdns())
    {
        LOG_ERR("Failed to initialize mDNS recovery advertisement");
        return false;
    }

    return true;
}
