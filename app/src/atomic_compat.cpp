/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/irq.h>

#include <cstdint>

#ifdef CONFIG_CPU_CORTEX_M0PLUS
extern "C" uint8_t __atomic_exchange_1(volatile void* ptr, uint8_t value, int)
{
    auto* const target = static_cast<volatile uint8_t*>(ptr);
    const auto  key    = irq_lock();
    const auto  old    = *target;

    *target = value;
    irq_unlock(key);

    return old;
}
#endif
