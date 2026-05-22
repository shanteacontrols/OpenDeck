/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/system/instance/impl/system.h"

#include <zephyr/logging/log.h>

using namespace opendeck;

namespace
{
    LOG_MODULE_REGISTER(opendeck_bootloader_system, LOG_LEVEL_INF);    // NOLINT
}    // namespace

bootloader::system::System::System(Hwa& hwa)
    : _hwa(hwa)
{}

bool bootloader::system::System::init()
{
    LOG_INF("OpenDeck bootloader starting");

    _hwa.init_indicators();

    if (_hwa.consume_staged_update())
    {
        _hwa.reboot_application();
    }

    if (!_hwa.init_webusb())
    {
        return false;
    }

    return _hwa.init_mdns();
}
