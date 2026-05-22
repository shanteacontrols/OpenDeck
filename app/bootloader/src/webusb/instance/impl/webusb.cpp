/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/webusb/instance/impl/webusb.h"

using namespace opendeck;

webusb::WebUsb::WebUsb(Hwa& hwa)
    : _hwa(hwa)
{
    bootloader::signaling::subscribe<bootloader::signaling::StatusSignal>(
        [this](const bootloader::signaling::StatusSignal& signal)
        {
            _hwa.status(signal.message());
        });
}

bool webusb::WebUsb::init()
{
    return _hwa.init();
}

bool webusb::WebUsb::deinit()
{
    return _hwa.deinit();
}
