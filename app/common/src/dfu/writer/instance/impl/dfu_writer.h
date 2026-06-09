/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/dfu_stream_parser/shared/common.h"
#include "common/src/dfu/flash_area/shared/deps.h"
#include "common/src/dfu/writer/shared/common.h"

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

namespace opendeck::common::dfu::writer
{
    /**
     * @brief Buffers parsed DFU payload bytes and writes aligned flash blocks.
     */
    class DfuWriter
    {
        public:
        /**
         * @brief Constructs a writer around a flash area backend.
         */
        explicit DfuWriter(opendeck::common::dfu::flash_area::Hwa& hwa);

        virtual ~DfuWriter() = default;

        /**
         * @brief Opens the underlying DFU flash area.
         *
         * @return `true` when the flash area is ready.
         */
        bool init();

        /**
         * @brief Starts a new payload write stream.
         *
         * @param header Raw DFU stream header accepted by the parser.
         * @param expected_size Number of firmware payload bytes expected.
         *
         * @return `true` on success, otherwise `false`.
         */
        virtual bool begin(const opendeck::common::dfu::dfu_stream_parser::Header& header, uint32_t expected_size);

        /**
         * @brief Appends payload bytes to the write stream.
         *
         * @param data Payload bytes.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool write(std::span<const uint8_t> data);

        /**
         * @brief Flushes pending bytes, padding the final block with erased bytes.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool finish();

        /**
         * @brief Resets cached write state.
         */
        virtual void abort();

        /**
         * @brief Reports whether this writer can accept DFU payloads.
         */
        virtual bool supported() const;

        /**
         * @brief Returns the selected buffered write block size.
         */
        size_t write_block_size() const;

        /**
         * @brief Returns writable payload capacity after the configured payload offset.
         */
        uint32_t capacity() const;

        protected:
        /**
         * @brief Resets the common writer state.
         */
        void reset();

        /**
         * @brief Selects an aligned buffered write size without starting a stream.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool prepare();

        /**
         * @brief Publishes one writer status message.
         *
         * @param message Status text.
         */
        void status(std::string_view message);

        /**
         * @brief Hook called after all payload bytes have been flushed.
         */
        virtual bool commit(const opendeck::common::dfu::dfu_stream_parser::Header& header, uint32_t expected_size);

        /**
         * @brief Hook called when an active transfer is aborted.
         */
        virtual void cancel();

        /**
         * @brief Returns the storage offset assigned to the first payload byte.
         */
        virtual uint32_t payload_offset() const;

        private:
        opendeck::common::dfu::flash_area::Hwa&          _hwa;
        opendeck::common::dfu::dfu_stream_parser::Header _header           = {};
        std::optional<size_t>                            _erased_sector    = std::nullopt;
        size_t                                           _write_block_size = 0;
        uint32_t                                         _expected_size    = 0;
        uint32_t                                         _bytes_written    = 0;
        uint32_t                                         _next_offset      = 0;
        uint32_t                                         _block_offset     = 0;
        size_t                                           _block_size       = 0;
        bool                                             _block_dirty      = false;
        bool                                             _active           = false;
        bool                                             _initialized      = false;
        std::array<uint8_t, MAX_FLASH_WRITE_BLOCK_SIZE>  _block_buffer     = {};

        /**
         * @brief Erases sectors overlapping the pending write range.
         */
        bool erase_range(uint32_t offset, size_t size);

        /**
         * @brief Appends one byte to the pending write block.
         */
        bool write_byte(uint8_t data);

        /**
         * @brief Writes one aligned block into the backend.
         */
        bool write_block(uint32_t offset, std::span<const uint8_t> data);
    };
}    // namespace opendeck::common::dfu::writer
