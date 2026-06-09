/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/flash_area/shared/deps.h"
#include <algorithm>
#include <array>
#include <span>
#include <vector>

namespace opendeck::common::dfu::flash_area
{
    /**
     * @brief In-memory flash-area backend used by tests.
     */
    class HwaTest : public Hwa
    {
        public:
        static constexpr uint32_t STORAGE_SIZE     = 4096U;
        static constexpr uint32_t SECTOR_SIZE      = 1024U;
        static constexpr size_t   WRITE_BLOCK_SIZE = 8U;

        HwaTest()
        {
            reset_storage();
        }

        bool open() override
        {
            _open_called = true;
            _opened      = _open_result;
            return _open_result;
        }

        bool opened() const override
        {
            return _opened;
        }

        uint32_t size() const override
        {
            return STORAGE_SIZE;
        }

        size_t write_block_size() const override
        {
            return _write_block_size;
        }

        bool read(uint32_t offset, std::span<uint8_t> data) const override
        {
            if (!_read_result || !range_valid(offset, data.size()))
            {
                return false;
            }

            std::copy(_storage.begin() + offset, _storage.begin() + offset + data.size(), data.begin());
            return true;
        }

        bool write(uint32_t offset, std::span<const uint8_t> data) const override
        {
            _write_calls++;

            if (!_write_result || !range_valid(offset, data.size()))
            {
                return false;
            }

            std::copy(data.begin(), data.end(), _storage.begin() + offset);
            return true;
        }

        bool erase(uint32_t offset, uint32_t size) const override
        {
            _erase_calls.push_back({
                .offset = offset,
                .size   = size,
            });

            if (!_erase_result || !range_valid(offset, size))
            {
                return false;
            }

            std::fill(_storage.begin() + offset, _storage.begin() + offset + size, opendeck::common::dfu::flash_area::ERASED_BYTE);
            return true;
        }

        std::optional<Sector> sector(const size_t index) const override
        {
            if (index >= this->sector_count())
            {
                return std::nullopt;
            }

            return this->sector_at(index);
        }

        bool sectors(std::span<Sector> sectors, size_t& sector_count) const override
        {
            if (sectors.size() < this->sector_count())
            {
                return false;
            }

            for (size_t i = 0; i < this->sector_count(); i++)
            {
                sectors[i] = sector_at(i);
            }

            sector_count = this->sector_count();
            return true;
        }

        Sector sector_at(size_t index) const
        {
            return {
                .offset = static_cast<uint32_t>(index * SECTOR_SIZE),
                .size   = SECTOR_SIZE,
            };
        }

        void reset_storage()
        {
            std::fill(_storage.begin(), _storage.end(), opendeck::common::dfu::flash_area::ERASED_BYTE);
            _erase_calls.clear();
            _write_calls      = 0;
            _open_called      = false;
            _opened           = false;
            _open_result      = true;
            _read_result      = true;
            _erase_result     = true;
            _write_result     = true;
            _write_block_size = WRITE_BLOCK_SIZE;
        }

        std::span<const uint8_t> storage() const
        {
            return _storage;
        }

        const std::vector<Sector>& erase_calls() const
        {
            return _erase_calls;
        }

        size_t write_calls() const
        {
            return _write_calls;
        }

        bool open_called() const
        {
            return _open_called;
        }

        void set_open_result(bool result)
        {
            _open_result = result;
        }

        void set_read_result(bool result)
        {
            _read_result = result;
        }

        void set_erase_result(bool result)
        {
            _erase_result = result;
        }

        void set_write_result(bool result)
        {
            _write_result = result;
        }

        void set_write_block_size(size_t size)
        {
            _write_block_size = size;
        }

        static constexpr size_t sector_count()
        {
            return STORAGE_SIZE / SECTOR_SIZE;
        }

        private:
        static constexpr bool range_valid(uint32_t offset, size_t size)
        {
            return (offset <= STORAGE_SIZE) &&
                   (size <= (STORAGE_SIZE - offset));
        }

        mutable std::array<uint8_t, STORAGE_SIZE> _storage          = {};
        mutable std::vector<Sector>               _erase_calls      = {};
        mutable size_t                            _write_calls      = 0;
        size_t                                    _write_block_size = WRITE_BLOCK_SIZE;
        bool                                      _open_called      = false;
        bool                                      _opened           = false;
        bool                                      _open_result      = true;
        bool                                      _read_result      = true;
        bool                                      _erase_result     = true;
        bool                                      _write_result     = true;
    };
}    // namespace opendeck::common::dfu::flash_area
