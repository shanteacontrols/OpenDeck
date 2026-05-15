/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

namespace opendeck::staged_update
{
    /**
     * @brief Magic value that marks a complete staged dfu.bin update.
     */
    static constexpr uint32_t METADATA_MAGIC = 0x5553444FU;    // "ODSU"

    /**
     * @brief Staged-update metadata format version.
     */
    static constexpr uint32_t METADATA_FORMAT_VERSION = 1U;

    /**
     * @brief Bytes reserved at the beginning of the staging partition.
     */
    static constexpr uint32_t METADATA_SIZE = 32U;

    /**
     * @brief Erased flash value used for currently unused metadata fields.
     */
    static constexpr uint32_t METADATA_RESERVED_VALUE = 0xFFFFFFFFU;

    /**
     * @brief Metadata written before the raw dfu.bin stream.
     */
    struct Metadata
    {
        uint32_t magic          = METADATA_MAGIC;
        uint32_t format_version = METADATA_FORMAT_VERSION;
        uint32_t target_uid     = 0;
        uint32_t size           = 0;
        uint32_t crc32          = 0;
        uint32_t reserved0      = METADATA_RESERVED_VALUE;
        uint32_t reserved1      = METADATA_RESERVED_VALUE;
        uint32_t reserved2      = METADATA_RESERVED_VALUE;
    };

    static_assert(sizeof(Metadata) == METADATA_SIZE);
}    // namespace opendeck::staged_update
