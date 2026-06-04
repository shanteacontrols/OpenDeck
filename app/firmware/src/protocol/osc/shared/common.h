/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace opendeck::firmware::protocol::osc
{
    /**
     * @brief OSC protocol settings stored in the global database block.
     */
    enum class Setting : uint8_t
    {
        Dest1Ipv4Octet0,
        Dest1Ipv4Octet1,
        Dest1Ipv4Octet2,
        Dest1Ipv4Octet3,
        Dest1Port,
        Dest2Ipv4Octet0,
        Dest2Ipv4Octet1,
        Dest2Ipv4Octet2,
        Dest2Ipv4Octet3,
        Dest2Port,
        Dest3Ipv4Octet0,
        Dest3Ipv4Octet1,
        Dest3Ipv4Octet2,
        Dest3Ipv4Octet3,
        Dest3Port,
        Dest4Ipv4Octet0,
        Dest4Ipv4Octet1,
        Dest4Ipv4Octet2,
        Dest4Ipv4Octet3,
        Dest4Port,
        ListenPort,
        RestrictIncomingToDestIp,
        Count
    };

    /** @brief Number of OSC destination endpoints stored in configuration. */
    constexpr inline size_t DESTINATION_COUNT = 4;

    /** @brief Default UDP destination port for outbound OSC packets. */
    constexpr inline uint16_t DEFAULT_DEST_PORT = 9000;

    /** @brief Default UDP listen port for inbound OSC packets. */
    constexpr inline uint16_t DEFAULT_LISTEN_PORT = 9001;

}    // namespace opendeck::firmware::protocol::osc
