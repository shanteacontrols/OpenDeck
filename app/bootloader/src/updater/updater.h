/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common.h"
#include "deps.h"

#include <array>

namespace opendeck::updater
{
    /**
     * @brief Consumes firmware update bytes and forwards validated chunks to the bootloader hardware layer.
     */
    class Updater
    {
        public:
        /**
         * @brief Constructs a firmware updater bound to the hardware abstraction.
         *
         * @param hwa Hardware abstraction used to store the incoming firmware image.
         */
        explicit Updater(Hwa& hwa);

        /**
         * @brief Processes one byte from the incoming firmware update stream.
         *
         * @param data Next byte to process.
         */
        void feed(uint8_t data);

        /**
         * @brief Resets the updater state machine and partial transfer state.
         */
        void reset();

        /**
         * @brief Returns whether the last processed stream completed successfully.
         *
         * @return `true` after a complete update stream triggered application.
         */
        bool completed() const;

        /**
         * @brief Returns whether the current or last processed stream failed validation.
         *
         * @return `true` when the updater rejected the stream or flash write failed.
         */
        bool failed() const;

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

        /**
         * @brief Reports whether the current parser step needs more data, completed, or failed validation.
         */
        enum class ProcessStatus : uint8_t
        {
            Complete,
            Incomplete,
            Invalid
        };

        /**
         * @brief Tracks one pending native flash write block.
         */
        struct WriteBlockCache
        {
            uint32_t offset       = 0;
            size_t   bytes_filled = 0;
            bool     dirty        = false;
        };

        static constexpr uint8_t ERASED_BYTE                = 0xFFU;
        static constexpr size_t  MAX_FLASH_WRITE_BLOCK_SIZE = 256;

        Hwa&                                            _hwa;
        uint8_t                                         _current_stage           = 0;
        size_t                                          _current_fw_page         = 0;
        uint32_t                                        _fw_page_bytes_received  = 0;
        uint8_t                                         _stage_bytes_received    = 0;
        uint32_t                                        _fw_bytes_received       = 0;
        uint32_t                                        _fw_size                 = 0;
        uint32_t                                        _received_uid            = 0;
        uint32_t                                        _received_format_version = 0;
        uint8_t                                         _start_bytes_received    = 0;
        size_t                                          _write_block_size        = 0;
        WriteBlockCache                                 _write_block_cache       = {};
        std::array<uint8_t, MAX_FLASH_WRITE_BLOCK_SIZE> _write_block_buffer      = {};
        bool                                            _failed                  = false;
        bool                                            _completed               = false;

        /**
         * @brief Processes the update-stream start marker.
         *
         * @param data Next byte from the update stream.
         *
         * @return Parser status after consuming the byte.
         */
        ProcessStatus process_start(uint8_t data);

        /**
         * @brief Processes firmware metadata bytes such as size and target UID.
         *
         * @param data Next byte from the update stream.
         *
         * @return Parser status after consuming the byte.
         */
        ProcessStatus process_fw_metadata(uint8_t data);

        /**
         * @brief Processes firmware payload bytes and writes completed chunks through the hardware layer.
         *
         * @param data Next byte from the update stream.
         *
         * @return Parser status after consuming the byte.
         */
        ProcessStatus process_fw_chunk(uint8_t data);

        /**
         * @brief Processes the update-stream end marker.
         *
         * @param data Next byte from the update stream.
         *
         * @return Parser status after consuming the byte.
         */
        ProcessStatus process_end(uint8_t data);

        /**
         * @brief Dispatches one byte to the current receive-stage parser.
         *
         * @param data Next byte from the update stream.
         *
         * @return Parser status after consuming the byte.
         */
        ProcessStatus process_current_stage(uint8_t data);

        /**
         * @brief Advances the parser to the next receive stage or applies a completed update.
         */
        void advance_stage();

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
         * @brief Finishes the current firmware page after flushing pending bytes.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool finish_current_page();

        /**
         * @brief Marks the update as failed and reports a status message.
         *
         * @param status Status text sent to the host.
         *
         * @return Always returns `false` for concise call sites.
         */
        bool fail(const char* status);
    };
}    // namespace opendeck::updater
