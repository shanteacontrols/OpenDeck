/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/dfu_stream_parser/instance/impl/dfu_stream_parser.h"
#include "common/src/dfu/upload/shared/common.h"

#include <cstdint>
#include <optional>
#include <span>

namespace opendeck::common::dfu::upload
{
    /**
     * @brief Transport-agnostic handler for command-based OpenDeck DFU uploads.
     */
    class Upload
    {
        public:
        /**
         * @brief Constructs the handler around a DFU writer.
         *
         * @param writer Writer for firmware payload bytes accepted by the DFU parser.
         */
        explicit Upload(writer::DfuWriter& writer);

        /**
         * @brief Parses the next firmware-upload frame from a byte stream prefix.
         *
         * @param data Bytes beginning at the next command boundary.
         *
         * @return Parsed frame information when the prefix starts with a supported command, otherwise
         *         `std::nullopt`.
         */
        static std::optional<Frame> frame_info(std::span<const uint8_t> data);

        /**
         * @brief Processes one firmware-upload command frame.
         *
         * @param data Transport payload containing one upload command.
         *
         * @return Upload command result when a command was handled, otherwise `std::nullopt`.
         */
        std::optional<CommandResult> handle(std::span<const uint8_t> data);

        /**
         * @brief Aborts any active upload and resets stream parsing state.
         */
        void abort();

        private:
        dfu_stream_parser::DfuStreamParser _dfu_stream;

        CommandResult result(Command command, Status status, bool finished = false) const;
        CommandResult handle_begin(const Frame& frame);
        CommandResult handle_chunk(const Frame& frame);
        CommandResult handle_finish(const Frame& frame);
        CommandResult handle_abort(const Frame& frame);
        Ack           make_ack(Command command, Status status) const;
    };
}    // namespace opendeck::common::dfu::upload
