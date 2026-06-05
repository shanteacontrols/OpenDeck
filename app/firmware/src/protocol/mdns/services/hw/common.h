/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string_view>

namespace opendeck::firmware::protocol::mdns
{
    /** @brief DNS-SD service type used by OpenDeck WebSockets. */
    constexpr inline std::string_view WEBSOCKETS_SERVICE = "_opendeck";

    /** @brief DNS-SD TXT record for the WebSockets endpoint. */
    constexpr inline char WEBSOCKETS_TXT[] = "\x0c"
                                             "path=/config";

    /** @brief DNS-SD service type used by OpenDeck OSC. */
    constexpr inline std::string_view OSC_SERVICE = "_opendeck-osc";

    /** @brief DNS-SD TXT record for the OSC endpoint. */
    constexpr inline char OSC_TXT[] = "\x08"
                                      "type=osc";
}    // namespace opendeck::firmware::protocol::mdns
