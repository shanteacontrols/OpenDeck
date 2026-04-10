/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: MIT
 */

#include "application/system/builder.h"

#include <zephyr/logging/log.h>

namespace
{
    LOG_MODULE_REGISTER(main, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
    static sys::Builder g_builder;
}    // namespace

int main()
{
    LOG_INF("Starting initialization");

    auto& system = g_builder.instance();

    if (!system.init())
    {
        return -1;
    }

    while (1)
    {
        system.run();
        k_msleep(1);
    }

    return 0;
}
