/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

#include <algorithm>
#include <array>
#include <vector>

namespace opendeck::staged_update
{
    /**
     * @brief In-memory staged DFU storage used by writer tests.
     */
    class HwaTest : public Hwa
    {
        public:
        struct EraseCall
        {
            uint32_t offset = 0;
            uint32_t size   = 0;
        };

        static constexpr uint8_t  ERASED_BYTE      = 0xFFU;
        static constexpr uint32_t STORAGE_SIZE     = 4096U;
        static constexpr uint32_t SECTOR_SIZE      = 1024U;
        static constexpr size_t   WRITE_BLOCK_SIZE = 8U;

        HwaTest()
        {
            reset_storage();
        }

        bool init() override
        {
            _init_called = true;
            return _init_result;
        }

        uint32_t size() const override
        {
            return STORAGE_SIZE;
        }

        size_t write_block_size() const override
        {
            return _write_block_size;
        }

        std::optional<Sector> sector(size_t index) const override
        {
            if (index >= sector_count())
            {
                return std::nullopt;
            }

            return Sector{
                .offset = static_cast<uint32_t>(index * SECTOR_SIZE),
                .size   = SECTOR_SIZE,
            };
        }

        bool erase(uint32_t offset, uint32_t size) override
        {
            _erase_calls.push_back({
                .offset = offset,
                .size   = size,
            });

            if (!_erase_result || !range_valid(offset, size))
            {
                return false;
            }

            std::fill(_storage.begin() + offset, _storage.begin() + offset + size, ERASED_BYTE);
            return true;
        }

        bool write(uint32_t offset, std::span<const uint8_t> data) override
        {
            _write_calls++;

            if (!_write_result || !range_valid(offset, data.size()))
            {
                return false;
            }

            std::copy(data.begin(), data.end(), _storage.begin() + offset);
            return true;
        }

        void reset_storage()
        {
            std::fill(_storage.begin(), _storage.end(), ERASED_BYTE);
            _erase_calls.clear();
            _write_calls      = 0;
            _init_called      = false;
            _init_result      = true;
            _erase_result     = true;
            _write_result     = true;
            _write_block_size = WRITE_BLOCK_SIZE;
        }

        std::span<const uint8_t> storage() const
        {
            return _storage;
        }

        const std::vector<EraseCall>& erase_calls() const
        {
            return _erase_calls;
        }

        size_t write_calls() const
        {
            return _write_calls;
        }

        bool init_called() const
        {
            return _init_called;
        }

        void set_init_result(bool result)
        {
            _init_result = result;
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

        private:
        static constexpr size_t sector_count()
        {
            return STORAGE_SIZE / SECTOR_SIZE;
        }

        static constexpr bool range_valid(uint32_t offset, size_t size)
        {
            return (offset <= STORAGE_SIZE) &&
                   (size <= (STORAGE_SIZE - offset));
        }

        std::array<uint8_t, STORAGE_SIZE> _storage          = {};
        std::vector<EraseCall>            _erase_calls      = {};
        size_t                            _write_calls      = 0;
        size_t                            _write_block_size = WRITE_BLOCK_SIZE;
        bool                              _init_result      = true;
        bool                              _erase_result     = true;
        bool                              _write_result     = true;
        bool                              _init_called      = false;
    };
}    // namespace opendeck::staged_update
