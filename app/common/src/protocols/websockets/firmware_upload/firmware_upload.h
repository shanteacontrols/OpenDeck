/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/dfu_stream_parser/instance/impl/dfu_stream_parser.h"
#include "common/src/protocols/websockets/shared/firmware_upload.h"

#include <cstdint>
#include <optional>
#include <span>

namespace opendeck::common::protocols::websockets
{
    /**
     * @brief Handles WebSocket frames that carry OpenDeck dfu.bin uploads.
     */
    class FirmwareUpload
    {
        public:
        /**
         * @brief Constructs the handler around a validated-payload destination.
         *
         * @param destination Destination for firmware payload bytes accepted by the DFU parser.
         */
        explicit FirmwareUpload(opendeck::common::dfu::dfu_stream_parser::Destination& destination);

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
        opendeck::common::dfu::dfu_stream_parser::Destination&    _destination;
        opendeck::common::dfu::dfu_stream_parser::DfuStreamParser _dfu_stream;

        FirmwareUploadCommandResult result(FirmwareUploadCommand command,
                                           FirmwareUploadStatus  status,
                                           bool                  finished = false) const;
        FirmwareUploadCommandResult handle_begin(std::span<const uint8_t> payload);
        FirmwareUploadCommandResult handle_chunk(std::span<const uint8_t> payload);
        FirmwareUploadCommandResult handle_finish();
        FirmwareUploadCommandResult handle_abort();
        FirmwareUploadAck           make_ack(FirmwareUploadCommand command, FirmwareUploadStatus status) const;
    };
}    // namespace opendeck::common::protocols::websockets
