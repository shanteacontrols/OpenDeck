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

    constexpr inline auto ROOT              = misc::StringLiteral{ "/opendeck" };
    constexpr inline auto SWITCH_COMPONENT  = misc::StringLiteral{ "/switch" };
    constexpr inline auto ENCODER_COMPONENT = misc::StringLiteral{ "/encoder" };
    constexpr inline auto ANALOG_COMPONENT  = misc::StringLiteral{ "/analog" };
    constexpr inline auto OUTPUT_COMPONENT  = misc::StringLiteral{ "/output" };
    constexpr inline auto RGB               = misc::StringLiteral{ "/rgb" };
    constexpr inline auto DISCOVER          = misc::StringLiteral{ "/discover" };
    constexpr inline auto DEVICE            = misc::StringLiteral{ "/device" };

    constexpr inline auto SWITCH      = misc::string_join(ROOT, SWITCH_COMPONENT);
    constexpr inline auto ENCODER     = misc::string_join(ROOT, ENCODER_COMPONENT);
    constexpr inline auto ANALOG      = misc::string_join(ROOT, ANALOG_COMPONENT);
    constexpr inline auto OUTPUT      = misc::string_join(ROOT, OUTPUT_COMPONENT);
    constexpr inline auto DISCOVERY   = misc::string_join(ROOT, DISCOVER);
    constexpr inline auto DEVICE_INFO = misc::string_join(ROOT, DEVICE);

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
