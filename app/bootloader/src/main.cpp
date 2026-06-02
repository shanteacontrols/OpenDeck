/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/system/builder/builder.h"

#include <zephyr/kernel.h>

using namespace opendeck::bootloader;

namespace
{
    system::Builder system_builder;
}    // namespace

int main()
{
    [[maybe_unused]] const bool initialized = system_builder.instance().init();

    while (true)
    {
        k_sleep(K_SECONDS(1));
    }
}
