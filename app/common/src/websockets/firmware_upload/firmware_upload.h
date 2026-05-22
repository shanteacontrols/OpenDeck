/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu_stream/instance/impl/dfu_stream.h"
#include "common/src/websockets/shared/firmware_upload.h"

#include <cstdint>
#include <optional>
#include <span>

namespace opendeck::websockets
{
    /**
     * @brief Handles WebSocket frames that carry OpenDeck dfu.bin uploads.
     */
    class FirmwareUpload
    {
        public:
        /**
         * @brief Constructs the handler around a validated-payload sink.
         *
         * @param sink Destination for firmware payload bytes accepted by the DFU parser.
         */
        explicit FirmwareUpload(dfu_stream::Sink& sink);

        /**
         * @brief Builds an ACK for a firmware-upload command.
         *
         * @param command Command being acknowledged.
         * @param status Result of the command.
         * @param bytes_written Number of accepted firmware payload bytes.
         *
         * @return Complete ACK frame.
         */
        static FirmwareUploadAck make_ack(FirmwareUploadCommand command,
                                          FirmwareUploadStatus  status,
                                          uint32_t              bytes_written);

        /**
         * @brief Processes one firmware-upload command frame.
         *
         * @param data Frame payload received from the WebSocket client.
         *
         * @return Upload command result when a command was handled, otherwise `std::nullopt`.
         */
        std::optional<FirmwareUploadCommandResult> handle(std::span<const uint8_t> data);

        /**
         * @brief Aborts any active upload and resets stream parsing state.
         */
        void abort();

        private:
        dfu_stream::Sink&     _sink;
        dfu_stream::DfuStream _dfu_stream;

        FirmwareUploadCommandResult result(FirmwareUploadCommand command,
                                           FirmwareUploadStatus  status,
                                           bool                  finished = false) const;
        FirmwareUploadCommandResult handle_begin(std::span<const uint8_t> payload);
        FirmwareUploadCommandResult handle_chunk(std::span<const uint8_t> payload);
        FirmwareUploadCommandResult handle_finish();
        FirmwareUploadCommandResult handle_abort();
        FirmwareUploadAck           make_ack(FirmwareUploadCommand command, FirmwareUploadStatus status) const;
    };
}    // namespace opendeck::websockets
