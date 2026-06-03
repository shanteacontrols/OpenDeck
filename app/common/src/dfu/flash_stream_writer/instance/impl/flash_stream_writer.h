/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/flash_stream_writer/instance/impl/common.h"
#include "common/src/dfu/flash_stream_writer/instance/impl/deps.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace opendeck::common::dfu::flash_stream_writer
{
    /**
     * @brief Buffers a byte stream into aligned flash write blocks.
     *
     * The writer accepts individual bytes, groups them into the configured
     * native write block size, pads the final partial block with erased bytes,
     * and forwards aligned blocks to a destination.
     */
    class FlashStreamWriter
    {
        public:
        /**
         * @brief Constructs a stream writer around a destination.
         *
         * @param destination Destination that receives aligned blocks.
         */
        explicit FlashStreamWriter(Destination& destination);

        /**
         * @brief Starts a new byte stream.
         *
         * @param write_block_size Required flash write block size.
         * @param start_offset Offset assigned to the first emitted block.
         *
         * @return `true` if the block size is supported, otherwise `false`.
         */
        bool begin(size_t write_block_size, uint32_t start_offset = 0);

        /**
         * @brief Appends one byte to the stream.
         *
         * @param data Byte to append.
         *
         * @return `true` if the byte was accepted and any required block write succeeded.
         */
        bool write(uint8_t data);

        /**
         * @brief Emits the pending partial block, padded with erased bytes.
         *
         * @return `true` if there is no pending data or the block write succeeded.
         */
        bool flush();

        /**
         * @brief Clears cached block state and disables writes until `begin()` is called again.
         */
        void reset();

        /**
         * @brief Returns the active flash write block size.
         *
         * @return Current write block size, or 0 before `begin()`.
         */
        size_t write_block_size() const;

        private:
        /**
         * @brief Tracks one partially filled flash write block.
         */
        struct WriteBlockCache
        {
            uint32_t offset       = 0;
            size_t   bytes_filled = 0;
            bool     dirty        = false;
        };

        Destination&                                    _destination;
        size_t                                          _write_block_size   = 0;
        uint32_t                                        _next_offset        = 0;
        WriteBlockCache                                 _write_block_cache  = {};
        std::array<uint8_t, MAX_FLASH_WRITE_BLOCK_SIZE> _write_block_buffer = {};
    };
}    // namespace opendeck::common::dfu::flash_stream_writer
