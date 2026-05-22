/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/dfu_stream/shared/common.h"

#include <cstdint>
#include <span>

namespace opendeck::common::dfu::dfu_stream
{
    /**
     * @brief Destination that receives validated firmware payload bytes from an OpenDeck DFU stream.
     */
    class Sink
    {
        public:
        virtual ~Sink() = default;

        /**
         * @brief Starts a new firmware payload transfer.
         *
         * @param header Raw DFU stream header accepted by the parser.
         * @param size Number of firmware payload bytes declared by the stream.
         *
         * @return `true` on success, otherwise `false`.
         */
        virtual bool begin(const Header& header, uint32_t size) = 0;

        /**
         * @brief Writes firmware payload bytes.
         *
         * @param data Validated payload bytes from the stream.
         *
         * @return `true` on success, otherwise `false`.
         */
        virtual bool write(std::span<const uint8_t> data) = 0;

        /**
         * @brief Commits the firmware payload after the stream end marker is accepted.
         *
         * @return `true` on success, otherwise `false`.
         */
        virtual bool finish() = 0;

        /**
         * @brief Cancels the active transfer after a rejected stream.
         */
        virtual void abort() = 0;
    };
}    // namespace opendeck::common::dfu::dfu_stream
