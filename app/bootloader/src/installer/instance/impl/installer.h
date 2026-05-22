/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/installer/shared/common.h"
#include "bootloader/src/installer/shared/deps.h"
#include "bootloader/src/staged_update_reader/shared/deps.h"
#include "common/src/flash_stream_writer/instance/impl/flash_stream_writer.h"

#include <string_view>

namespace opendeck::installer
{
    /**
     * @brief Consumes firmware update bytes and forwards accepted chunks to the bootloader hardware layer.
     */
    class Installer : public staged_update_reader::Consumer, private flash_stream_writer::Sink
    {
        public:
        /**
         * @brief Constructs a firmware installer bound to the hardware abstraction.
         *
         * @param hwa Hardware abstraction used to store the incoming firmware image.
         */
        explicit Installer(Hwa& hwa);

        /**
         * @brief Processes one byte from the incoming firmware update stream.
         *
         * @param data Next byte to process.
         */
        staged_update_reader::StreamStatus feed(uint8_t data) override;

        /**
         * @brief Resets the installer state machine and partial transfer state.
         */
        void reset() override;

        private:
        /**
         * @brief Identifies the current section of the firmware update stream being parsed.
         */
        enum class ReceiveStage : uint8_t
        {
            Start,
            FwMetadata,
            FwChunk,
            End,
            Count
        };

        using StreamStatus = staged_update_reader::StreamStatus;

        Hwa&                                   _hwa;
        flash_stream_writer::FlashStreamWriter _writer;
        uint8_t                                _current_stage           = 0;
        size_t                                 _current_sector          = 0;
        uint32_t                               _sector_bytes_received   = 0;
        uint8_t                                _stage_bytes_received    = 0;
        uint32_t                               _fw_bytes_received       = 0;
        uint32_t                               _fw_size                 = 0;
        uint32_t                               _received_uid            = 0;
        uint32_t                               _received_format_version = 0;
        uint8_t                                _start_bytes_received    = 0;
        size_t                                 _write_block_size        = 0;
        bool                                   _failed                  = false;

        /**
         * @brief Processes the update-stream start marker.
         *
         * @param data Next byte from the update stream.
         *
         * @return Parser status after consuming the byte.
         */
        StreamStatus process_start(uint8_t data);

        /**
         * @brief Processes firmware metadata bytes such as size and target UID.
         *
         * @param data Next byte from the update stream.
         *
         * @return Parser status after consuming the byte.
         */
        StreamStatus process_fw_metadata(uint8_t data);

        /**
         * @brief Processes firmware payload bytes and writes completed chunks through the hardware layer.
         *
         * @param data Next byte from the update stream.
         *
         * @return Parser status after consuming the byte.
         */
        StreamStatus process_fw_chunk(uint8_t data);

        /**
         * @brief Processes the update-stream end marker.
         *
         * @param data Next byte from the update stream.
         *
         * @return Parser status after consuming the byte.
         */
        StreamStatus process_end(uint8_t data);

        /**
         * @brief Dispatches one byte to the current receive-stage parser.
         *
         * @param data Next byte from the update stream.
         *
         * @return Parser status after consuming the byte.
         */
        StreamStatus process_current_stage(uint8_t data);

        /**
         * @brief Advances the parser to the next receive stage or applies a completed update.
         *
         * @return Parser status after advancing.
         */
        StreamStatus advance_stage();

        /**
         * @brief Resets parser state and optionally clears the latched write failure.
         *
         * @param clear_failure When `true`, clears the failure latch too.
         */
        void reset_state(bool clear_failure);

        /**
         * @brief Validates and caches the backend write-block size.
         *
         * @return `true` when the write-block size can be handled, otherwise `false`.
         */
        bool prepare_write_block_size();

        /**
         * @brief Stores one payload byte in the pending native write block.
         *
         * @param data Payload byte to stage.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool write_payload_byte(uint8_t data);

        /**
         * @brief Flushes the pending native write block, padding unused bytes as erased.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool flush_write_block();

        /**
         * @brief Writes one aligned block to the current firmware sector.
         */
        bool write_block(uint32_t offset, std::span<const uint8_t> data) override;

        /**
         * @brief Finishes the current firmware sector after flushing pending bytes.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool finish_current_sector();

        /**
         * @brief Marks the update as failed and reports a status message.
         *
         * @param status Status text sent to the host.
         *
         * @return Always returns `false` for concise call sites.
         */
        bool fail(std::string_view status);

        /**
         * @brief Publishes one installer status message.
         *
         * @param message Status text sent to bootloader status subscribers.
         */
        void status(std::string_view message);
    };
}    // namespace opendeck::installer
