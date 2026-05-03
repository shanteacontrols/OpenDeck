/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <zephyr/kernel.h>

namespace opendeck::util
{
    /**
     * @brief Sleeps the current thread or yields immediately when no sleep is requested.
     *
     * @param sleep_ms Sleep time in milliseconds. A value of `0` yields to other
     *                 ready threads instead of sleeping on wall-clock time.
     */
    inline void thread_sleep(int sleep_ms)
    {
        if (sleep_ms == 0)
        {
            k_yield();
        }
        else
        {
            k_msleep(sleep_ms);
        }
    }
}    // namespace opendeck::util
