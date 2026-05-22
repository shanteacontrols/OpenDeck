/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace opendeck::common::dfu::dfu_stream
{
    /**
     * @brief Magic value that marks the beginning of an OpenDeck DFU stream.
     */
    constexpr inline uint32_t START_COMMAND = OPENDECK_DFU_BEGIN_MAGIC;

    /**
     * @brief Supported OpenDeck DFU stream format version.
     */
    constexpr inline uint32_t FORMAT_VERSION = OPENDECK_DFU_FORMAT_VERSION;

    /**
     * @brief Magic value that marks the end of an OpenDeck DFU stream.
     */
    constexpr inline uint32_t END_COMMAND = OPENDECK_DFU_END_MAGIC;

    /**
     * @brief Bytes before the firmware payload in an OpenDeck DFU stream.
     */
    constexpr inline size_t HEADER_SIZE = sizeof(uint32_t) * 4U;

    /**
     * @brief Raw OpenDeck DFU stream header bytes.
     */
    using Header = std::array<uint8_t, HEADER_SIZE>;

    /**
     * @brief Result of feeding bytes into an OpenDeck DFU stream parser.
     */
    enum class StreamStatus : uint8_t
    {
        Complete,
        Incomplete,
        Invalid
    };
}    // namespace opendeck::common::dfu::dfu_stream
