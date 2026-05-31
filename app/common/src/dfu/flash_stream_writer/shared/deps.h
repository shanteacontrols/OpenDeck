/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <span>

namespace opendeck::common::dfu::flash_stream_writer
{
    /**
     * @brief Destination for aligned flash write blocks produced by `FlashStreamWriter`.
     */
    class Destination
    {
        public:
        virtual ~Destination() = default;

        /**
         * @brief Writes one flash-aligned block.
         *
         * @param offset Absolute or destination-local byte offset for this block.
         * @param data Block bytes. Size is the write block size passed to `FlashStreamWriter::begin()`.
         *
         * @return `true` on success, otherwise `false`.
         */
        virtual bool write_block(uint32_t offset, std::span<const uint8_t> data) = 0;
    };
}    // namespace opendeck::common::dfu::flash_stream_writer
