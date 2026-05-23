/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "zlibs/utils/threads/threads.h"

#include <cstddef>

namespace opendeck::bootloader::threads
{
    /** @brief Stack size, in bytes, assigned to the WebSocket DFU client handler. */
    constexpr inline size_t WEBSOCKETS_CLIENT_THREAD_STACK_SIZE = 4096;

    /**
     * @brief Thread type used for bootloader WebSocket DFU client handling.
     */
    using WebSocketsClientThread = zlibs::utils::threads::UserThread<zlibs::utils::misc::StringLiteral{ "boot_websocket" },
                                                                     K_PRIO_PREEMPT(1),
                                                                     WEBSOCKETS_CLIENT_THREAD_STACK_SIZE>;
}    // namespace opendeck::bootloader::threads
