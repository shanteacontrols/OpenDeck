/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string_view>

namespace opendeck::bootloader::protocols::mdns
{
    /** @brief DNS-SD service type used by OpenDeck bootloader DFU. */
    constexpr inline std::string_view DFU_SERVICE = "_opendeck-dfu";

    /** @brief TXT record advertised with the OpenDeck bootloader DFU service. */
    constexpr inline char DFU_TXT[] = "\x09"
                                      "path=/dfu";
}    // namespace opendeck::bootloader::protocols::mdns
