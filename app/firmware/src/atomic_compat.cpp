/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/irq.h>

#include <cstdint>

#ifdef CONFIG_CPU_CORTEX_M0PLUS
/**
 * @brief Provides the missing byte-sized atomic exchange builtin for Cortex-M0+ targets.
 *
 * Zephyr firmware code and linked libraries can emit calls to `__atomic_exchange_1`, but
 * Cortex-M0+ toolchains do not always provide a lock-free implementation in the runtime.
 * This shim keeps the link successful and preserves the expected exchange semantics by
 * performing the read-modify-write sequence with interrupts locked around it.
 *
 * @param ptr Address of the byte to exchange.
 * @param value New byte value to store.
 * @param Unused memory-order argument required by the builtin ABI.
 *
 * @return Previous byte value stored at `ptr`.
 */

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" uint8_t __atomic_exchange_1(volatile void*       target_ptr,
                                       uint8_t              new_value,
                                       [[maybe_unused]] int memory_order)
{
    auto* const target_pointer = static_cast<volatile uint8_t*>(target_ptr);
    const auto  irq_key        = irq_lock();
    const auto  old_value      = *target_pointer;

    *target_pointer = new_value;
    irq_unlock(irq_key);

    return old_value;
}
#endif
