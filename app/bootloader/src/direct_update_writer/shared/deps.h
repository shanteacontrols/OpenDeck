/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <span>
#include <stddef.h>
#include <stdint.h>

namespace opendeck::direct_update_writer
{
    /**
     * @brief Hardware abstraction used by the bootloader direct-update writer.
     */
    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        /**
         * @brief Returns the writable firmware slot size.
         *
         * @return Slot size in bytes.
         */
        virtual uint32_t size() = 0;

        /**
         * @brief Returns the size of one writable flash sector.
         *
         * @param index Sector index to query.
         *
         * @return Sector size in bytes, or `0` when the sector is unavailable.
         */
        virtual uint32_t sector_size(size_t index) = 0;

        /**
         * @brief Returns the required flash write granularity.
         *
         * @return Write block size in bytes.
         */
        virtual size_t write_block_size() = 0;

        /**
         * @brief Erases one target flash sector.
         *
         * @param index Sector index to erase.
         *
         * @return `true` on success, otherwise `false`.
         */
        virtual bool erase_sector(size_t index) = 0;

        /**
         * @brief Writes one native flash block into the selected sector.
         *
         * @param index Sector index being prepared.
         * @param offset Byte offset within the sector.
         * @param data Bytes to write. Offset and size must be aligned to `write_block_size()`.
         *
         * @return `true` on success, otherwise `false`.
         */
        virtual bool write_sector(size_t index, uint32_t offset, std::span<const uint8_t> data) = 0;

        /**
         * @brief Applies the completed update, typically by rebooting or swapping images.
         */
        virtual void apply() = 0;
    };
}    // namespace opendeck::direct_update_writer
