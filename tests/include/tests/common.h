/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "io/base.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <vector>
#include <functional>
#include <string>
#include <cstddef>
#include <iostream>

using namespace ::testing;

namespace tests
{
    inline void resume_io()
    {
        io::Base::resume();
        k_msleep(20);
    }

    /**
     * @brief Polls until a condition becomes true or a timeout expires.
     *
     * @param [in] condition Predicate evaluated between sleep intervals.
     * @param [in] timeout_ms Maximum time to wait before returning.
     * @param [in] poll_ms Delay between condition checks.
     *
     * @return `true` if the condition became true within the timeout window,
     *         otherwise the final condition value after the timeout expires.
     */
    template<typename Condition>
    bool wait_until(Condition condition, const int32_t timeout_ms = 200, const int32_t poll_ms = 20)
    {
        for (int32_t elapsed_ms = 0; elapsed_ms < timeout_ms; elapsed_ms += poll_ms)
        {
            if (condition())
            {
                return true;
            }

            k_msleep(poll_ms);
        }

        return condition();
    }
}    // namespace tests
