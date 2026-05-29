/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/irq.h>

#include <cstdint>

#ifdef CONFIG_CPU_CORTEX_M0PLUS
namespace
{
    template<typename T>
    T atomic_exchange_irq_locked(volatile void* target_ptr, T new_value)
    {
        auto* const target_pointer = static_cast<volatile T*>(target_ptr);
        const auto  irq_key        = irq_lock();
        const auto  old_value      = *target_pointer;

        *target_pointer = new_value;
        irq_unlock(irq_key);

        return old_value;
    }
}    // namespace

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" uint8_t __atomic_exchange_1(volatile void*       target_ptr,
                                       uint8_t              new_value,
                                       [[maybe_unused]] int memory_order)
{
    return atomic_exchange_irq_locked(target_ptr, new_value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" uint32_t __atomic_exchange_4(volatile void*       target_ptr,
                                        uint32_t             new_value,
                                        [[maybe_unused]] int memory_order)
{
    return atomic_exchange_irq_locked(target_ptr, new_value);
}
#endif
