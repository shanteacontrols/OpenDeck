/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace opendeck::protocol::osc
{
    /** @brief Maximum size of an encoded OSC packet. */
    constexpr inline size_t PACKET_BUFFER_SIZE = 192;

    /** @brief Maximum number of OSC arguments accepted by the packet parser. */
    constexpr inline size_t MAX_ARGUMENT_COUNT = 8;

    /** @brief Fixed buffer used for one encoded OSC packet. */
    using PacketBuffer = std::array<uint8_t, PACKET_BUFFER_SIZE>;

    /**
     * @brief OSC 32-bit integer argument.
     */
    struct OscInt32
    {
        using type = int32_t;    // NOLINT(readability-identifier-naming)

        type                  value    = 0;
        static constexpr char TYPE_TAG = 'i';
    };

    /**
     * @brief OSC string argument.
     */
    struct OscString
    {
        using type = std::string_view;    // NOLINT(readability-identifier-naming)

        type                  value    = {};
        static constexpr char TYPE_TAG = 's';
    };

    /**
     * @brief OSC 32-bit floating-point argument.
     */
    struct OscFloat32
    {
        using type = float;    // NOLINT(readability-identifier-naming)

        type                  value    = 0.0F;
        static constexpr char TYPE_TAG = 'f';
    };

    /**
     * @brief OSC address made from a fixed prefix and numeric component index.
     *
     * For example, `{ paths::OUTPUT.c_str(), 3 }` becomes `/opendeck/output/3`.
     */
    struct OscIndexedAddress
    {
        std::string_view prefix = {};
        size_t           index  = 0;
    };
}    // namespace opendeck::protocol::osc
