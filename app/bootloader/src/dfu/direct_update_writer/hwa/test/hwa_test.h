/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/direct_update_writer/shared/deps.h"

#include <algorithm>
#include <span>
#include <vector>

namespace opendeck::bootloader::dfu::direct_update_writer
{
    /**
     * @brief Test direct-update writer backend backed by an in-memory flash sector image.
     */
    class HwaTest : public Hwa
    {
        public:
        explicit HwaTest(size_t write_block_size = sizeof(uint32_t),
                         size_t sector_size      = BOOTLOADER_TEST_FLASH_SECTOR_SIZE,
                         size_t sector_count     = 1)
            : _write_block_size(write_block_size)
            , _sector_size(sector_size)
            , _sectors(sector_count)
            , _written_sectors(sector_count, false)
        {}

        /**
         * @brief Returns the writable emulated firmware slot size.
         *
         * @return Emulated slot size in bytes.
         */
        uint32_t size() override
        {
            return static_cast<uint32_t>(_sector_size * _sectors.size());
        }

        /**
         * @brief Returns the size of the emulated flash sector.
         *
         * @param index Sector index to query.
         *
         * @return Emulated sector size for sector `0`, otherwise `0`.
         */
        uint32_t sector_size(size_t index) override
        {
            return index < _sectors.size() ? _sector_size : 0;
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
         * @brief Erases the emulated sector by filling it with erased bytes.
         *
         * @param index Sector index to erase.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool erase_sector(size_t index) override
        {
            if ((index >= _sectors.size()) || fail_erase)
            {
                return false;
            }

            _sectors[index].assign(_sector_size, 0xFF);
            _written_sectors[index] = false;
            rebuild_written_bytes();
            erased_sectors.push_back(index);
            return true;
        }

        /**
         * @brief Stores one flash-aligned write block in the emulated sector image.
         *
         * @param index Sector index being prepared.
         * @param address Byte offset within the sector.
         * @param data Bytes to store.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool write_sector(size_t index, uint32_t address, std::span<const uint8_t> data) override
        {
            if ((index >= _sectors.size()) ||
                fail_write ||
                data.empty() ||
                ((data.size() % _write_block_size) != 0) ||
                ((address % _write_block_size) != 0) ||
                (data.size() > _sectors[index].size()) ||
                (address > (_sectors[index].size() - data.size())))
            {
                return false;
            }

            std::copy(data.begin(), data.end(), _sectors[index].begin() + address);
            _written_sectors[index] = true;
            rebuild_written_bytes();
            write_count++;
            return true;
        }

        /**
         * @brief Marks the test backend as updated.
         */
        void apply() override
        {
            updated = true;
        }

        bool                 updated        = false;
        bool                 fail_erase     = false;
        bool                 fail_write     = false;
        size_t               write_count    = 0;
        std::vector<size_t>  erased_sectors = {};
        std::vector<uint8_t> written_bytes  = {};

        private:
        void rebuild_written_bytes()
        {
            written_bytes.clear();

            for (size_t sector = 0; sector < _sectors.size(); sector++)
            {
                if (_written_sectors[sector])
                {
                    written_bytes.insert(written_bytes.end(), _sectors[sector].begin(), _sectors[sector].end());
                }
            }
        }

        size_t                            _write_block_size = sizeof(uint32_t);
        size_t                            _sector_size      = BOOTLOADER_TEST_FLASH_SECTOR_SIZE;
        std::vector<std::vector<uint8_t>> _sectors          = {};
        std::vector<bool>                 _written_sectors  = {};
    };
}    // namespace opendeck::bootloader::dfu::direct_update_writer
