/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/system/builder/builder.h"

#include <zephyr/logging/log.h>

using namespace opendeck;

namespace
{
    LOG_MODULE_REGISTER(main, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
    sys::Builder builder;
}    // namespace

int main()
{
    auto& system = builder.instance();

    if (!system.init())
    {
        return -1;
    }

    return 0;
}
