/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/direct_update_writer/shared/deps.h"
#include "common/src/dfu_stream/shared/common.h"
#include "common/src/dfu_stream/shared/deps.h"
#include "common/src/flash_stream_writer/instance/impl/flash_stream_writer.h"

#include <string_view>

namespace opendeck::direct_update_writer
{
    /**
     * @brief Writes validated firmware payload bytes to the application slot.
     */
    class DirectUpdateWriter : public dfu_stream::Sink, private flash_stream_writer::Sink
    {
        public:
        /**
         * @brief Constructs a direct-update writer bound to the hardware abstraction.
         *
         * @param hwa Hardware abstraction used to store the incoming firmware image.
         */
        explicit DirectUpdateWriter(Hwa& hwa);

        /**
         * @brief Starts installing a validated firmware payload.
         *
         * @param header Raw DFU stream header accepted by the parser.
         * @param size Firmware payload size in bytes.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool begin(const dfu_stream::Header& header, uint32_t size) override;

        /**
         * @brief Writes validated firmware payload bytes.
         *
         * @param data Firmware payload bytes.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool write(std::span<const uint8_t> data) override;

        /**
         * @brief Applies the installed firmware payload.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool finish() override;

        /**
         * @brief Aborts the active firmware payload install.
         */
        void abort() override;

        /**
         * @brief Resets the direct-update writer state machine and partial transfer state.
         */
        void reset();

        private:
        Hwa&                                   _hwa;
        flash_stream_writer::FlashStreamWriter _writer;
        size_t                                 _current_sector        = 0;
        uint32_t                               _sector_bytes_received = 0;
        size_t                                 _write_block_size      = 0;
        bool                                   _active                = false;
        bool                                   _failed                = false;

        /**
         * @brief Resets writer state and optionally clears the latched write failure.
         *
         * @param clear_failure When `true`, clears the failure latch too.
         */
        void reset_state(bool clear_failure);

        /**
         * @brief Checks whether the firmware payload can fit in the destination slot.
         *
         * @param size Firmware payload size in bytes.
         *
         * @return `true` when enough slot space is available, otherwise `false`.
         */
        bool payload_fits(uint32_t size);

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
         * @brief Publishes one direct-update writer status message.
         *
         * @param message Status text sent to bootloader status subscribers.
         */
        void status(std::string_view message);
    };
}    // namespace opendeck::direct_update_writer
