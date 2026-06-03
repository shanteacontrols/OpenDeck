/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/io/indicators/instance/impl/indicators.h"

using namespace opendeck::bootloader;

io::indicators::Indicators::Indicators(Hwa& hwa)
    : _hwa(hwa)
    , _blink_work([this]()
                  {
                      blink();
                  })
{
    bootloader::signaling::subscribe<common::signaling::FirmwareUpdateStartedSignal>(
        [this](const common::signaling::FirmwareUpdateStartedSignal&)
        {
            start_blinking_all();
        });
}

bool io::indicators::Indicators::init()
{
    if (_configured)
    {
        return true;
    }

    _configured = _hwa.init();

    if (_configured)
    {
        _blink_work.cancel();
        _blink_on = true;
        _hwa.on();
    }

    return _configured;
}

void io::indicators::Indicators::start_blinking_all()
{
    if (!_configured)
    {
        return;
    }

    _blink_on = false;
    _hwa.off();
    _blink_work.reschedule(0);
}

void io::indicators::Indicators::blink()
{
    _blink_on = !_blink_on;
    _blink_on ? _hwa.on() : _hwa.off();
    _blink_work.reschedule(BLINK_INTERVAL_MS);
}
