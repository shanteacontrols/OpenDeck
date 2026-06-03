/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/protocols/webusb/instance/impl/webusb.h"

using namespace opendeck::bootloader::protocols;

webusb::WebUsb::WebUsb(Hwa& hwa)
    : _hwa(hwa)
{
    opendeck::bootloader::signaling::subscribe<opendeck::common::signaling::DfuStatusSignal>(
        [this](const opendeck::common::signaling::DfuStatusSignal& signal)
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
