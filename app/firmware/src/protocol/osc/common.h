/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace opendeck::protocol::osc
{
    /**
     * @brief OSC protocol settings stored in the global database block.
     */
    enum class Setting : uint8_t
    {
        Enable,
        DestIpv4Octet0,
        DestIpv4Octet1,
        DestIpv4Octet2,
        DestIpv4Octet3,
        DestPort,
        ListenPort,
        RestrictIncomingToDestIp,
        Count
    };

    /** @brief Default UDP destination port for outbound OSC packets. */
    constexpr inline uint16_t DEFAULT_DEST_PORT = 9000;

    /** @brief Default UDP listen port for inbound OSC packets. */
    constexpr inline uint16_t DEFAULT_LISTEN_PORT = 9001;

    /** @brief Largest valid UDP port value. */
    constexpr inline uint32_t UDP_PORT_MAX = UINT16_MAX;

    /** @brief Largest valid IPv4 octet value. */
    constexpr inline uint32_t IPV4_OCTET_MAX = UINT8_MAX;

    /** @brief Maximum number of pending OSC TX events buffered by the send worker thread. */
    constexpr inline size_t TX_QUEUE_SIZE = 32;

    /** @brief Delay before retrying a failed OSC listen socket setup. */
    constexpr inline int LISTEN_SOCKET_RETRY_DELAY_MS = 100;
}    // namespace opendeck::protocol::osc
