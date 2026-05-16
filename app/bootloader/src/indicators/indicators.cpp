/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/indicators/indicators.h"
#include "firmware/src/io/indicators/common.h"
#include "firmware/src/io/indicators/hwa/hw/hwa_hw.h"

#include "zlibs/utils/misc/kwork_delayable.h"

using namespace opendeck;

namespace
{
    constexpr uint32_t    BLINK_INTERVAL_MS = 50U;
    io::indicators::HwaHw hwa;
    bool                  configured = false;
    bool                  blink_on   = false;

    void blink_handler();

    zlibs::utils::misc::KworkDelayable blink_work(blink_handler);

    void blink_handler()
    {
        blink_on = !blink_on;
        blink_on ? hwa.on(io::indicators::Type::All) : hwa.off(io::indicators::Type::All);
        blink_work.reschedule(BLINK_INTERVAL_MS);
    }
}    // namespace

bool indicators::init()
{
    if (configured)
    {
        return true;
    }

    configured = hwa.init();

    if (configured)
    {
        blink_work.cancel();
        blink_on = true;
        hwa.on(io::indicators::Type::All);
    }

    return configured;
}

void indicators::start_blinking_all()
{
    if (!configured)
    {
        return;
    }

    blink_on = false;
    hwa.off(io::indicators::Type::All);
    blink_work.reschedule(0);
}
