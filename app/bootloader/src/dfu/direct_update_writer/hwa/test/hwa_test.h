/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/direct_update_writer/instance/impl/deps.h"
#include "common/src/dfu/flash_area/impl/deps.h"
#include "common/src/dfu/flash_area/shared/common.h"

#include <algorithm>
#include <span>
#include <vector>

namespace opendeck::bootloader::dfu::direct_update_writer
{
    /**
     * @brief Test direct-update writer backend backed by an in-memory flash sector image.
     */
    class HwaTest : public opendeck::common::dfu::flash_area::Hwa, public Hwa
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
        bool open(uint8_t) override
        {
            _opened = true;
            return true;
        }

        bool opened() const override
        {
            return _opened;
        }

        uint32_t size() const override
        {
            return static_cast<uint32_t>(_sector_size * _sectors.size());
        }

        size_t write_block_size() const override
        {
            return _write_block_size;
        }

        bool read(uint32_t offset, std::span<uint8_t> data) const override
        {
            if (!range_valid(offset, data.size()))
            {
                return false;
            }

            const size_t index         = offset / _sector_size;
            const size_t sector_offset = offset % _sector_size;

            if ((index >= _sectors.size()) || (data.size() > (_sector_size - sector_offset)))
            {
                return false;
            }

            std::copy(_sectors[index].begin() + sector_offset,
                      _sectors[index].begin() + sector_offset + data.size(),
                      data.begin());

            return true;
        }

        bool write(uint32_t offset, std::span<const uint8_t> data) const override
        {
            if (fail_write ||
                data.empty() ||
                ((data.size() % _write_block_size) != 0) ||
                ((offset % _write_block_size) != 0) ||
                !range_valid(offset, data.size()))
            {
                return false;
            }

            const size_t index         = offset / _sector_size;
            const size_t sector_offset = offset % _sector_size;

            if ((index >= _sectors.size()) || (data.size() > (_sector_size - sector_offset)))
            {
                return false;
            }

            std::copy(data.begin(), data.end(), _sectors[index].begin() + sector_offset);
            _written_sectors[index] = true;
            rebuild_written_bytes();
            write_count++;
            return true;
        }

        bool erase(uint32_t offset, uint32_t size) const override
        {
            if (fail_erase || !range_valid(offset, size) || ((offset % _sector_size) != 0) || ((size % _sector_size) != 0))
            {
                return false;
            }

            const size_t first = offset / _sector_size;
            const size_t count = size / _sector_size;

            for (size_t i = 0; i < count; i++)
            {
                const size_t index = first + i;

                if (index >= _sectors.size())
                {
                    return false;
                }

                _sectors[index].assign(_sector_size, 0xFF);
                _written_sectors[index] = false;
                erased_sectors.push_back(index);
            }

            rebuild_written_bytes();
            return true;
        }

        std::optional<opendeck::common::dfu::flash_area::Sector> sector(size_t index) const override
        {
            if (index >= _sectors.size())
            {
                return std::nullopt;
            }

            return opendeck::common::dfu::flash_area::Sector{
                .offset = static_cast<uint32_t>(index * _sector_size),
                .size   = static_cast<uint32_t>(_sector_size),
            };
        }

        bool sectors(std::span<opendeck::common::dfu::flash_area::Sector> sectors, size_t& sector_count) const override
        {
            if (sectors.size() < _sectors.size())
            {
                return false;
            }

            for (size_t i = 0; i < _sectors.size(); i++)
            {
                sectors[i] = *sector(i);
            }

            sector_count = _sectors.size();
            return true;
        }

        /**
         * @brief Marks the test backend as updated.
         */
        void apply() override
        {
            updated = true;
        }

        bool                         updated        = false;
        bool                         fail_erase     = false;
        bool                         fail_write     = false;
        mutable size_t               write_count    = 0;
        mutable std::vector<size_t>  erased_sectors = {};
        mutable std::vector<uint8_t> written_bytes  = {};

        private:
        bool range_valid(const uint32_t offset, const size_t size) const
        {
            return (offset <= this->size()) &&
                   (size <= (this->size() - offset));
        }

        void rebuild_written_bytes() const
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

        size_t                                    _write_block_size = sizeof(uint32_t);
        size_t                                    _sector_size      = BOOTLOADER_TEST_FLASH_SECTOR_SIZE;
        mutable std::vector<std::vector<uint8_t>> _sectors          = {};
        mutable std::vector<bool>                 _written_sectors  = {};
        bool                                      _opened           = false;
    };
}    // namespace opendeck::bootloader::dfu::direct_update_writer
