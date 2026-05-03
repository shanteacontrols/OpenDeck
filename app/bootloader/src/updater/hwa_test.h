/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

#include <algorithm>
#include <span>
#include <vector>

namespace opendeck::updater
{
    /**
     * @brief Test updater backend backed by an in-memory flash page image.
     */
    class HwaTest : public Hwa
    {
        public:
        explicit HwaTest(size_t write_block_size = sizeof(uint32_t),
                         size_t page_size        = BOOTLOADER_TEST_FLASH_PAGE_SIZE,
                         size_t page_count       = 1)
            : _write_block_size(write_block_size)
            , _page_size(page_size)
            , _pages(page_count)
            , _written_pages(page_count, false)
        {}

        /**
         * @brief Returns the size of the emulated flash page.
         *
         * @param index Page index to query.
         *
         * @return Emulated page size for page `0`, otherwise `0`.
         */
        uint32_t page_size(size_t index) override
        {
            return index < _pages.size() ? _page_size : 0;
        }

        /**
         * @brief Returns the emulated native write-block size.
         *
         * @return Write-block size in bytes.
         */
        size_t write_block_size() override
        {
            return _write_block_size;
        }

        /**
         * @brief Erases the emulated page by filling it with erased bytes.
         *
         * @param index Page index to erase.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool erase_page(size_t index) override
        {
            if ((index >= _pages.size()) || fail_erase)
            {
                return false;
            }

            _pages[index].assign(_page_size, 0xFF);
            _written_pages[index] = false;
            rebuild_written_bytes();
            erased_pages.push_back(index);
            return true;
        }

        /**
         * @brief Stores one native write block in the emulated page image.
         *
         * @param index Page index being prepared.
         * @param address Byte offset within the page.
         * @param data Bytes to store.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool write_page(size_t index, uint32_t address, std::span<const uint8_t> data) override
        {
            if ((index >= _pages.size()) ||
                fail_write ||
                (data.size() != _write_block_size) ||
                ((address % _write_block_size) != 0) ||
                (data.size() > _pages[index].size()) ||
                (address > (_pages[index].size() - data.size())))
            {
                return false;
            }

            std::copy(data.begin(), data.end(), _pages[index].begin() + address);
            _written_pages[index] = true;
            rebuild_written_bytes();
            write_count++;
            return true;
        }

        /**
         * @brief Performs no cleanup in the test backend.
         */
        void cleanup() override
        {}

        /**
         * @brief Marks the test backend as updated.
         */
        void apply() override
        {
            updated = true;
        }

        /**
         * @brief Performs no action when an update session starts.
         */
        void on_firmware_update_start() override
        {}

        bool                 updated       = false;
        bool                 fail_erase    = false;
        bool                 fail_write    = false;
        size_t               write_count   = 0;
        std::vector<size_t>  erased_pages  = {};
        std::vector<uint8_t> written_bytes = {};

        private:
        void rebuild_written_bytes()
        {
            written_bytes.clear();

            for (size_t page = 0; page < _pages.size(); page++)
            {
                if (_written_pages[page])
                {
                    written_bytes.insert(written_bytes.end(), _pages[page].begin(), _pages[page].end());
                }
            }
        }

        size_t                            _write_block_size = sizeof(uint32_t);
        size_t                            _page_size        = BOOTLOADER_TEST_FLASH_PAGE_SIZE;
        std::vector<std::vector<uint8_t>> _pages            = {};
        std::vector<bool>                 _written_pages    = {};
    };
}    // namespace opendeck::updater
