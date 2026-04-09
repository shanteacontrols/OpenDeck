/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <span>
#include <stddef.h>
#include <stdint.h>

namespace updater
{
    /**
     * @brief Callback invoked before the updater hands control back to the platform.
     */
    using CleanupCallback = void (*)();

    /**
     * @brief Hardware abstraction used by the bootloader firmware updater.
     */
    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        /**
         * @brief Returns the size of one writable flash page.
         *
         * @param index Page index to query.
         *
         * @return Page size in bytes, or `0` when the page is unavailable.
         */
        virtual uint32_t page_size(size_t index) = 0;

        /**
         * @brief Returns the required flash write granularity.
         *
         * @return Write block size in bytes.
         */
        virtual size_t write_block_size() = 0;

        /**
         * @brief Erases one target flash page.
         *
         * @param index Page index to erase.
         *
         * @return `true` on success, otherwise `false`.
         */
        virtual bool erase_page(size_t index) = 0;

        /**
         * @brief Writes one native flash block into the selected page.
         *
         * @param index Page index being prepared.
         * @param offset Byte offset within the page.
         * @param data Bytes to write. Size must match `write_block_size()`.
         *
         * @return `true` on success, otherwise `false`.
         */
        virtual bool write_page(size_t index, uint32_t offset, std::span<const uint8_t> data) = 0;

        /**
         * @brief Performs any platform-specific cleanup before leaving the updater.
         */
        virtual void cleanup() = 0;

        /**
         * @brief Applies the completed update, typically by rebooting or swapping images.
         */
        virtual void apply() = 0;

        /**
         * @brief Notifies the platform that firmware update processing is starting.
         */
        virtual void on_firmware_update_start() = 0;
    };
}    // namespace updater
