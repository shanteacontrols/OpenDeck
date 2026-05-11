/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "zlibs/utils/misc/string.h"

#include <cstddef>
#include <optional>
#include <string_view>

namespace opendeck::protocol::osc::paths
{
    namespace misc = zlibs::utils::misc;

    constexpr inline auto ROOT     = misc::StringLiteral{ "/opendeck" };
    constexpr inline auto INPUT    = misc::StringLiteral{ "/input" };
    constexpr inline auto OUTPUT   = misc::StringLiteral{ "/output" };
    constexpr inline auto BUTTON   = misc::StringLiteral{ "/button" };
    constexpr inline auto ENCODER  = misc::StringLiteral{ "/encoder" };
    constexpr inline auto ANALOG   = misc::StringLiteral{ "/analog" };
    constexpr inline auto LED      = misc::StringLiteral{ "/led" };
    constexpr inline auto RGB      = misc::StringLiteral{ "/rgb" };
    constexpr inline auto DISCOVER = misc::StringLiteral{ "/discover" };
    constexpr inline auto DEVICE   = misc::StringLiteral{ "/device" };

    constexpr inline auto INPUT_BUTTON  = misc::string_join(ROOT, INPUT, BUTTON);
    constexpr inline auto INPUT_ENCODER = misc::string_join(ROOT, INPUT, ENCODER);
    constexpr inline auto INPUT_ANALOG  = misc::string_join(ROOT, INPUT, ANALOG);
    constexpr inline auto OUTPUT_LED    = misc::string_join(ROOT, OUTPUT, LED);
    constexpr inline auto DISCOVERY     = misc::string_join(ROOT, DISCOVER);
    constexpr inline auto DEVICE_INFO   = misc::string_join(ROOT, DEVICE);

    /**
     * @brief Extracts a component index from an address with a fixed OpenDeck path prefix.
     *
     * @param address OSC address pattern.
     * @param prefix Address prefix ending before the component index.
     *
     * @return Component index, or empty when the address does not match.
     */
    inline std::optional<size_t> parse_indexed(std::string_view address, std::string_view prefix)
    {
        if ((address.size() <= (prefix.size() + 1U)) ||
            (address.substr(0, prefix.size()) != prefix) ||
            (address[prefix.size()] != '/'))
        {
            return {};
        }

        size_t index = 0;

        for (size_t i = prefix.size() + 1U; i < address.size(); i++)
        {
            const char c = address[i];

            if ((c < '0') || (c > '9'))
            {
                return {};
            }

            index = (index * 10U) + static_cast<size_t>(c - '0');
        }

        return index;
    }
}    // namespace opendeck::protocol::osc::paths
