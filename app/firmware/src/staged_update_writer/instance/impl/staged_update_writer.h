/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/staged_update_writer/shared/deps.h"
#include "common/src/staged_update/shared/common.h"
#include "common/src/flash_stream_writer/instance/impl/flash_stream_writer.h"

#include <span>

namespace opendeck::staged_update_writer
{
    /**
     * @brief Stores a raw dfu.bin stream in the configured staging partition.
     */
    class StagedUpdateWriter : private flash_stream_writer::Sink
    {
        public:
        /**
         * @brief Constructs staged-update storage around a flash backend.
         *
         * @param hwa Flash storage backend used by staged update.
         */
        explicit StagedUpdateWriter(Hwa& hwa);

        /**
         * @brief Erases the staging partition and starts a new upload.
         *
         * @param expected_size Number of raw dfu.bin bytes expected.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool begin(uint32_t expected_size);

        /**
         * @brief Appends bytes to the staged raw dfu.bin stream.
         *
         * @param data Upload chunk to append.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool write(std::span<const uint8_t> data);

        /**
         * @brief Commits staging metadata after the upload completed.
         *
         * @return `true` if the staged stream is complete and committed, otherwise `false`.
         */
        bool finish();

        /**
         * @brief Invalidates staged update metadata and resets writer state.
         */
        void abort();

        /**
         * @brief Returns the number of payload bytes written so far.
         *
         * @return Current staged dfu.bin byte count.
         */
        uint32_t bytes_written() const;

        /**
         * @brief Returns the usable staged dfu.bin capacity.
         *
         * @return Maximum number of raw dfu.bin bytes that fit after metadata.
         */
        uint32_t capacity() const;

        private:
        Hwa&                                   _hwa;
        flash_stream_writer::FlashStreamWriter _writer;
        uint32_t                               _expected_size = 0;
        uint32_t                               _bytes_written = 0;
        uint32_t                               _crc           = 0;
        bool                                   _initialized   = false;
        bool                                   _active        = false;

        /**
         * @brief Opens the staged DFU partition and reads its flash layout.
         */
        bool init_flash_area();

        /**
         * @brief Erases the whole staged DFU partition before a new upload.
         */
        bool erase_partition();

        /**
         * @brief Erases the first sector so the bootloader ignores any old upload.
         */
        bool erase_metadata_sector();

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
         * @brief Writes the metadata marker after the full dfu.bin stream is stored.
         */
        bool write_metadata();

        /**
         * @brief Clears upload counters and cached write state.
         */
        void reset_state();
    };
}    // namespace opendeck::staged_update_writer
