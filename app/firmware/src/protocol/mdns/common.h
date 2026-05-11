/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace opendeck::protocol::mdns
{
    /** @brief Bytes reserved in the database for the custom hostname label, including the terminator. */
    constexpr inline size_t CUSTOM_HOSTNAME_DB_SIZE = 64;

    /** @brief Suffix appended to the hostname label for mDNS names. */
    constexpr inline std::string_view LOCAL_SUFFIX = ".local";

    /** @brief Bytes needed for the full advertised mDNS name, including the terminator. */
    constexpr inline size_t NETWORK_NAME_SIZE = CUSTOM_HOSTNAME_DB_SIZE + LOCAL_SUFFIX.size();

    /** @brief Bytes needed for an IPv4 address string, including the terminator. */
    constexpr inline size_t IPV4_ADDRESS_SIZE = 16;

    /** @brief Base hostname used before OpenDeck appends target and serial metadata. */
    constexpr inline std::string_view HOSTNAME_PREFIX = "opendeck";

    /** @brief DNS-SD service type used by OpenDeck WebConfig. */
    constexpr inline std::string_view WEBCONFIG_SERVICE = "_opendeck";

    /** @brief DNS-SD protocol label for WebConfig. */
    constexpr inline std::string_view WEBCONFIG_PROTOCOL = "_tcp";

    /** @brief DNS-SD service type used by OpenDeck OSC. */
    constexpr inline std::string_view OSC_SERVICE = "_opendeck-osc";

    /** @brief DNS-SD protocol label for OSC. */
    constexpr inline std::string_view OSC_PROTOCOL = "_udp";

    /** @brief DNS-SD domain used by mDNS. */
    constexpr inline std::string_view LOCAL_DOMAIN = "local";

    /** @brief DNS-SD TXT record for the WebConfig endpoint. */
    constexpr inline char WEBCONFIG_TXT[] = "\x0c"
                                            "path=/config";

    /** @brief Hardware ID bytes used in the default hostname suffix. */
    constexpr inline size_t SERIAL_SUFFIX_BYTES = 4;
}    // namespace opendeck::protocol::mdns
