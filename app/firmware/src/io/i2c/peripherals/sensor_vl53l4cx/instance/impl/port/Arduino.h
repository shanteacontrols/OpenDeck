/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>
#include <zephyr/kernel.h>

// NOLINTBEGIN(readability-identifier-naming)

#define OUTPUT 1
#define INPUT  0
#define HIGH   1
#define LOW    0

inline void pinMode([[maybe_unused]] int pin, [[maybe_unused]] int mode)
{
}

inline void digitalWrite([[maybe_unused]] int pin, [[maybe_unused]] int value)
{
}

inline void delay(uint32_t ms)
{
    k_sleep(K_MSEC(ms));
}

inline void delayMicroseconds(uint32_t us)
{
    k_sleep(K_USEC(us));
}

inline uint32_t millis()
{
    return k_uptime_get_32();
}

// NOLINTEND(readability-identifier-naming)
