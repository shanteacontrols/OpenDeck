/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

namespace opendeck::firmware::protocol::osc
{
    /**
     * @brief OSC protocol settings stored in the global database block.
     */
    enum class Setting : uint8_t
    {
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

}    // namespace opendeck::firmware::protocol::osc
