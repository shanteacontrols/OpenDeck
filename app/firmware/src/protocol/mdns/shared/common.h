/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/mdns/shared/common.h"

namespace opendeck::protocol::mdns
{
    /** @brief DNS-SD service type used by OpenDeck WebSockets. */
    constexpr inline std::string_view WEBSOCKETS_SERVICE = "_opendeck";

    /** @brief DNS-SD TXT record for the WebSockets endpoint. */
    constexpr inline char WEBSOCKETS_TXT[] = "\x0c"
                                             "path=/config";

    /** @brief DNS-SD service type used by OpenDeck OSC. */
    constexpr inline std::string_view OSC_SERVICE = "_opendeck-osc";
}    // namespace opendeck::protocol::mdns
