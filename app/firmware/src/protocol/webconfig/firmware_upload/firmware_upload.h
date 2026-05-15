/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/webconfig/firmware_upload/common.h"
#include "firmware/src/staged_update/staged_update.h"

#include <array>
#include <cstdint>
#include <optional>
#include <span>

namespace opendeck::protocol::webconfig
{
    /**
     * @brief Handles WebConfig frames that upload dfu.bin into staged-update storage.
     */
    class FirmwareUploadHandler
    {
        public:
        /**
         * @brief Response sent after one firmware-upload command.
         */
        using Response = std::array<uint8_t, FIRMWARE_UPLOAD_ACK_SIZE>;

        /**
         * @brief Constructs the handler around staged-update storage.
         *
         * @param staged_update Staged-update storage used for firmware uploads.
         */
        explicit FirmwareUploadHandler(staged_update::StagedUpdate& staged_update);

        /**
         * @brief Builds an ACK for a firmware-upload command.
         *
         * @param command Command being acknowledged.
         * @param status Result of the command.
         * @param bytes_written Number of dfu.bin bytes stored so far.
         *
         * @return Complete ACK frame.
         */
        static Response make_ack(FirmwareUploadCommand command, FirmwareUploadStatus status, uint32_t bytes_written);

        /**
         * @brief Processes one firmware-upload command frame.
         *
         * @param data Frame payload received from the browser.
         *
         * @return ACK frame when a command was handled, otherwise `std::nullopt`.
         */
        std::optional<Response> handle(std::span<const uint8_t> data);

        /**
         * @brief Reports whether the last frame completed an upload.
         *
         * @return `true` once after a successful finish command.
         */
        bool take_reboot_request();

        private:
        staged_update::StagedUpdate& _staged_update;
        bool                         _reboot_requested = false;

        /**
         * @brief Starts a new staged firmware upload.
         *
         * @param payload Begin-frame payload containing the total dfu.bin byte count.
         *
         * @return ACK frame for the begin command.
         */
        Response handle_firmware_begin(std::span<const uint8_t> payload);

        /**
         * @brief Writes one upload chunk.
         *
         * @param payload Raw dfu.bin bytes for this chunk.
         *
         * @return ACK frame for the chunk command.
         */
        Response handle_firmware_chunk(std::span<const uint8_t> payload);

        /**
         * @brief Commits the upload after all bytes have been written.
         *
         * @return ACK frame for the finish command.
         */
        Response handle_firmware_finish();

        /**
         * @brief Aborts the active staged firmware upload.
         *
         * @return ACK frame for the abort command.
         */
        Response handle_firmware_abort();

        /**
         * @brief Builds an ACK using the current staged-update byte count.
         *
         * @param command Command being acknowledged.
         * @param status Result of the command.
         *
         * @return Complete ACK frame.
         */
        Response make_ack(FirmwareUploadCommand command, FirmwareUploadStatus status) const;
    };
}    // namespace opendeck::protocol::webconfig
