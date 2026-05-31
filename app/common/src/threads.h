/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "zlibs/utils/threads/threads.h"

#include <cstddef>

namespace opendeck::common::threads
{
    /** @brief Stack size, in bytes, assigned to WebSocket client handling. */
    constexpr inline size_t WEBSOCKETS_THREAD_STACK_SIZE = 4096;

    /** @brief Stack size, in bytes, assigned to WebSocket frame transmission. */
    constexpr inline size_t WEBSOCKETS_TX_THREAD_STACK_SIZE = 4096;

    /**
     * @brief Thread type used for WebSocket client handling.
     */
    using WebSocketsThread = zlibs::utils::threads::UserThread<zlibs::utils::misc::StringLiteral{ "websocket" },
                                                               K_PRIO_PREEMPT(1),
                                                               WEBSOCKETS_THREAD_STACK_SIZE>;

    /**
     * @brief Thread type used for WebSocket frame transmission.
     */
    using WebSocketsTxThread = zlibs::utils::threads::UserThread<zlibs::utils::misc::StringLiteral{ "websocket_tx" },
                                                                 K_PRIO_PREEMPT(1),
                                                                 WEBSOCKETS_TX_THREAD_STACK_SIZE>;
}    // namespace opendeck::common::threads
