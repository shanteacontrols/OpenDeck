/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/dfu_stream/shared/deps.h"

#include <cstddef>
#include <cstdint>
#include <span>

namespace opendeck::bootloader::dfu::staged_update_reader
{
    /**
     * @brief Hardware abstraction used to access staged firmware payload storage.
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
         * @brief Returns the native write-block size of staged DFU storage.
         *
         * @return Native write-block size in bytes.
         */
        virtual size_t write_block_size() const = 0;

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
         *
         * @return `true` when the marker was invalidated, otherwise `false`.
         */
        virtual bool clear_pending() = 0;
    };
}    // namespace opendeck::bootloader::dfu::staged_update_reader
