/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace opendeck::mdns
{
    /** @brief Bytes reserved for a custom hostname label, including the terminator. */
    constexpr inline size_t CUSTOM_HOSTNAME_SIZE = 64;

    /** @brief Suffix appended to the hostname label for mDNS names. */
    constexpr inline std::string_view LOCAL_SUFFIX = ".local";

    /** @brief Bytes needed for the full advertised mDNS name, including the terminator. */
    constexpr inline size_t NETWORK_NAME_SIZE = CUSTOM_HOSTNAME_SIZE + LOCAL_SUFFIX.size();

    /** @brief Bytes needed for an IPv4 address string, including the terminator. */
    constexpr inline size_t IPV4_ADDRESS_SIZE = 16;

    /** @brief Base hostname used before OpenDeck appends target and serial metadata. */
    constexpr inline std::string_view HOSTNAME_PREFIX = "opendeck";

    /** @brief DNS-SD protocol label for TCP services. */
    constexpr inline std::string_view TCP_PROTOCOL = "_tcp";

    /** @brief DNS-SD protocol label for UDP services. */
    constexpr inline std::string_view UDP_PROTOCOL = "_udp";

    /** @brief DNS-SD domain used by mDNS. */
    constexpr inline std::string_view LOCAL_DOMAIN = "local";

    /**
     * @brief Mutable DNS-SD service record fields updated at runtime.
     */
    struct Service
    {
        std::span<char> instance;
        uint16_t&       port;
        uint16_t        service_port = 0;
    };
}    // namespace opendeck::mdns
