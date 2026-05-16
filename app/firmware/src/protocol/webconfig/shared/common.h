/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace opendeck::protocol::webconfig
{
    /** @brief TCP port used by the WebSocket configuration endpoint. */
    constexpr inline uint16_t DEFAULT_PORT = 80;

    /** @brief HTTP/WebSocket slots reserved so a new client can replace the active client. */
    constexpr inline size_t CLIENT_COUNT = 2;

    /** @brief WebSocket receive buffer size used by Zephyr's HTTP upgrade handler. */
    constexpr inline size_t UPGRADE_BUFFER_SIZE = 256;

    /** @brief Maximum binary WebConfig frame size accepted by the endpoint. */
    constexpr inline size_t FRAME_BUFFER_SIZE = 2304;
}    // namespace opendeck::protocol::webconfig
