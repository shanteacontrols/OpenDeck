/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/dfu_stream/shared/common.h"
#include "common/src/dfu/dfu_stream/shared/deps.h"

#include <cstdint>
#include <span>

namespace opendeck::common::dfu::dfu_stream
{
    /**
     * @brief Parses an OpenDeck dfu.bin stream and forwards firmware payload bytes to a sink.
     */
    class DfuStream
    {
        public:
        /**
         * @brief Constructs the parser around a payload sink.
         *
         * @param sink Destination for accepted firmware payload bytes.
         */
        explicit DfuStream(Sink& sink);

        /**
         * @brief Reads the firmware payload size declared by a DFU header.
         *
         * @param header Raw DFU stream header.
         *
         * @return Declared payload size.
         */
        static uint32_t payload_size(const Header& header);

        /**
         * @brief Checks whether a DFU header targets this firmware image.
         *
         * @param header Raw DFU stream header.
         *
         * @return `true` when the header is supported by this target.
         */
        static bool header_valid(const Header& header);

        /**
         * @brief Resets parser state and forgets any partial stream.
         */
        void reset();

        /**
         * @brief Processes one dfu.bin byte.
         *
         * @param data Next byte from the dfu.bin stream.
         *
         * @return Parser status after consuming the byte.
         */
        StreamStatus feed(uint8_t data);

        /**
         * @brief Processes dfu.bin bytes.
         *
         * @param data Next bytes from the dfu.bin stream.
         *
         * @return Parser status after consuming the bytes.
         */
        StreamStatus feed(std::span<const uint8_t> data);

        /**
         * @brief Returns the current parser status.
         *
         * @return Last parser status.
         */
        StreamStatus status() const;

        /**
         * @brief Returns firmware payload bytes accepted by the sink.
         *
         * @return Number of accepted payload bytes.
         */
        uint32_t bytes_written() const;

        /**
         * @brief Returns declared firmware payload size after metadata is accepted.
         *
         * @return Declared payload size, or zero before metadata is accepted.
         */
        uint32_t expected_size() const;

        private:
        enum class ReceiveStage : uint8_t
        {
            Start,
            Metadata,
            Payload,
            End,
            Done,
            Count
        };

        Sink&        _sink;
        ReceiveStage _stage                   = ReceiveStage::Start;
        StreamStatus _status                  = StreamStatus::Incomplete;
        uint8_t      _stage_bytes_received    = 0;
        uint32_t     _received_format_version = 0;
        uint32_t     _received_uid            = 0;
        uint32_t     _expected_size           = 0;
        uint32_t     _bytes_written           = 0;
        Header       _header                  = {};
        bool         _sink_active             = false;

        StreamStatus process_start(uint8_t data);
        StreamStatus process_metadata(uint8_t data);
        StreamStatus process_payload(uint8_t data);
        StreamStatus process_end(uint8_t data);
        StreamStatus process_current_stage(uint8_t data);
        StreamStatus advance_stage();
        StreamStatus reject();
    };
}    // namespace opendeck::common::dfu::dfu_stream
