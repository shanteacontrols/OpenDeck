/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/dfu_stream_parser/shared/deps.h"
#include "common/src/dfu/flash_stream_writer/instance/impl/flash_stream_writer.h"
#include "firmware/src/dfu/staged_update_writer/instance/impl/deps.h"

#include <optional>
#include <span>

namespace opendeck::firmware::dfu::staged_update_writer
{
    /**
     * @brief Stores a validated firmware payload in the configured staging partition.
     */
    class StagedUpdateWriter : public opendeck::common::dfu::dfu_stream_parser::Destination, private opendeck::common::dfu::flash_stream_writer::Destination
    {
        public:
        /**
         * @brief Constructs staged-update storage around a flash backend.
         *
         * @param hwa Flash storage backend used by staged update.
         */
        explicit StagedUpdateWriter(Hwa& hwa);

        /**
         * @brief Invalidates any old staged update and starts a new upload.
         *
         * @param header Raw DFU header accepted by the parser.
         * @param expected_size Number of firmware payload bytes expected.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool begin(const opendeck::common::dfu::dfu_stream_parser::Header& header, uint32_t expected_size) override;

        /**
         * @brief Appends bytes to the staged firmware payload.
         *
         * @param data Upload chunk to append.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool write(std::span<const uint8_t> data) override;

        /**
         * @brief Flushes staged payload bytes after the upload completed.
         *
         * @return `true` if the staged stream is complete and committed, otherwise `false`.
         */
        bool finish() override;

        /**
         * @brief Reports whether staged-update storage is supported.
         *
         * @return `true` if the backend supports staged updates.
         */
        bool supported() const override;

        /**
         * @brief Invalidates staged update header and resets writer state.
         */
        void abort() override;

        private:
        Hwa&                                                          _hwa;
        opendeck::common::dfu::flash_stream_writer::FlashStreamWriter _writer;
        opendeck::common::dfu::dfu_stream_parser::Header              _header        = {};
        uint32_t                                                      _expected_size = 0;
        std::optional<size_t>                                         _erased_sector = std::nullopt;
        bool                                                          _initialized   = false;
        bool                                                          _active        = false;

        /**
         * @brief Opens the staged DFU partition and reads its flash layout.
         */
        bool init_flash_area();

        /**
         * @brief Returns the usable staged firmware payload capacity.
         *
         * @return Maximum firmware payload bytes that fit after the DFU header.
         */
        uint32_t staging_capacity() const;

        /**
         * @brief Returns the flash-aligned space reserved for the DFU header.
         *
         * @return Header storage size in bytes.
         */
        uint32_t header_storage_size() const;

        /**
         * @brief Erases the first sector so the bootloader ignores any old upload.
         */
        bool erase_header_sector();

        /**
         * @brief Erases staged DFU sectors that overlap a pending payload write.
         */
        bool erase_payload_range(uint32_t offset, size_t size);

        /**
         * @brief Writes one flash-aligned block into the staged DFU partition.
         */
        bool write_block(uint32_t offset, std::span<const uint8_t> data) override;

        /**
         * @brief Adds one byte to the pending flash block and updates the CRC.
         */
        bool append_stream_byte(uint8_t data);

        /**
         * @brief Writes the cached block to flash, padding the final block if needed.
         */
        bool flush_write_block();

        /**
         * @brief Writes the accepted DFU header at the beginning of the staged partition.
         */
        bool write_header(const opendeck::common::dfu::dfu_stream_parser::Header& header);

        /**
         * @brief Clears upload counters and cached write state.
         */
        void reset_state();
    };
}    // namespace opendeck::firmware::dfu::staged_update_writer
