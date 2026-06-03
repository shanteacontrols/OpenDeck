/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/flash_area/hwa/test/hwa_test.h"
#include "firmware/src/dfu/staged_update_writer/instance/impl/deps.h"

#include <optional>
#include <span>
#include <vector>

namespace opendeck::firmware::dfu::staged_update_writer
{
    /**
     * @brief In-memory staged DFU storage used by writer tests.
     */
    class HwaTest : public Hwa
    {
        public:
        bool init() override
        {
            return _area.open(0);
        }

        uint32_t size() const override
        {
            return _area.size();
        }

        size_t write_block_size() const override
        {
            return _area.write_block_size();
        }

        std::optional<Sector> sector(size_t index) const override
        {
            if (index >= opendeck::common::dfu::flash_area::HwaTest::sector_count())
            {
                return std::nullopt;
            }

            const auto sector = _area.sector(index);

            return Sector{
                .offset = sector->offset,
                .size   = sector->size,
            };
        }

        bool erase(uint32_t offset, uint32_t size) override
        {
            return _area.erase(offset, size);
        }

        bool write(uint32_t offset, std::span<const uint8_t> data) override
        {
            return _area.write(offset, data);
        }

        void reset_storage()
        {
            _area.reset_storage();
        }

        std::span<const uint8_t> storage() const
        {
            return _area.storage();
        }

        const std::vector<opendeck::common::dfu::flash_area::HwaTest::EraseCall>& erase_calls() const
        {
            return _area.erase_calls();
        }

        size_t write_calls() const
        {
            return _area.write_calls();
        }

        bool init_called() const
        {
            return _area.open_called();
        }

        void set_init_result(bool result)
        {
            _area.set_open_result(result);
        }

        void set_erase_result(bool result)
        {
            _area.set_erase_result(result);
        }

        void set_write_result(bool result)
        {
            _area.set_write_result(result);
        }

        void set_write_block_size(size_t size)
        {
            _area.set_write_block_size(size);
        }

        private:
        opendeck::common::dfu::flash_area::HwaTest _area;
    };
}    // namespace opendeck::firmware::dfu::staged_update_writer
