/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <span>

namespace opendeck::staged_update_reader
{
    /**
     * @brief Result of feeding one byte into a staged-update consumer.
     */
    enum class StreamStatus : uint8_t
    {
        Complete,
        Incomplete,
        Invalid
    };

    /**
     * @brief Destination that accepts dfu.bin bytes streamed by the staged-update reader.
     */
    class Consumer
    {
        public:
        virtual ~Consumer() = default;

        /**
         * @brief Prepares the destination for a new staged update stream.
         */
        virtual void reset() = 0;

        /**
         * @brief Accepts one dfu.bin byte from staged storage.
         *
         * @param byte Next byte from staged storage.
         *
         * @return Consumer status after processing the byte.
         */
        virtual StreamStatus feed(uint8_t byte) = 0;
    };

    /**
     * @brief Hardware abstraction used to access staged DFU storage.
     */
    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        /**
         * @brief Opens staged DFU storage.
         *
         * @return `true` when staged storage is available, otherwise `false`.
         */
        virtual bool init() = 0;

        /**
         * @brief Returns staged DFU storage capacity in bytes.
         *
         * @return Storage capacity in bytes.
         */
        virtual uint32_t size() const = 0;

        /**
         * @brief Reads bytes from staged DFU storage.
         *
         * @param offset Byte offset within staged storage.
         * @param data Destination buffer.
         *
         * @return `true` on success, otherwise `false`.
         */
        virtual bool read(uint32_t offset, std::span<uint8_t> data) = 0;

        /**
         * @brief Invalidates the staged-update marker.
         */
        virtual void clear_pending() = 0;
    };
}    // namespace opendeck::staged_update_reader
