/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "zlibs/utils/threads/threads.h"

#include <cstddef>

namespace opendeck::bootloader::threads
{
    /** @brief Stack size, in bytes, assigned to WebUSB DFU RX processing. */
    constexpr inline size_t WEBUSB_RX_THREAD_STACK_SIZE = 4096;

    /**
     * @brief Thread type used for WebUSB DFU RX processing.
     */
    using WebUsbRxThread = zlibs::utils::threads::UserThread<zlibs::utils::misc::StringLiteral{ "webusb_rx" },
                                                             K_PRIO_PREEMPT(1),
                                                             WEBUSB_RX_THREAD_STACK_SIZE>;
}    // namespace opendeck::bootloader::threads
