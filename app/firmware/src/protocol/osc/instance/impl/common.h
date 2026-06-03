/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace opendeck::firmware::protocol::osc
{
    /** @brief Largest valid UDP port value. */
    constexpr inline uint32_t UDP_PORT_MAX = UINT16_MAX;

    /** @brief Largest valid IPv4 octet value. */
    constexpr inline uint32_t IPV4_OCTET_MAX = UINT8_MAX;

    /** @brief Maximum number of pending OSC TX events buffered by the send worker thread. */
    constexpr inline size_t TX_QUEUE_SIZE = 32;

    /** @brief Delay before retrying a failed OSC listen socket setup. */
    constexpr inline int LISTEN_SOCKET_RETRY_DELAY_MS = 100;
}    // namespace opendeck::firmware::protocol::osc
