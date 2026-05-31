/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace opendeck::io::indicators
{
    constexpr inline uint32_t TRAFFIC_INDICATOR_TIMEOUT_MS       = 50;
    constexpr inline uint32_t FACTORY_RESET_INDICATOR_TIMEOUT_MS = 250;
    constexpr inline uint32_t STARTUP_INDICATOR_TIMEOUT_MS       = 150;
    constexpr inline size_t   STARTUP_INDICATOR_FLASH_COUNT      = 3;
}    // namespace opendeck::io::indicators
