/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

namespace opendeck::common::system
{
    /**
     * @brief Delay before the system reboots into the selected firmware target.
     */
    constexpr inline uint32_t REBOOT_DELAY_MS = 1000;
}    // namespace opendeck::common::system
