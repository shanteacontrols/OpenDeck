/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/flash_area/hwa/test/hwa_test.h"
#include "firmware/src/dfu/staged_update_writer/instance/impl/deps.h"

namespace opendeck::firmware::dfu::staged_update_writer
{
    /**
     * @brief In-memory staged DFU storage used by writer tests.
     */
    class HwaTest : public Hwa
    {
        public:
        opendeck::common::dfu::flash_area::Hwa& flash_area() override
        {
            return _area;
        }

        bool init_called() const
        {
            return _area.open_called();
        }

        void set_init_result(bool result)
        {
            _area.set_open_result(result);
        }

        std::span<const uint8_t> storage() const
        {
            return _area.storage();
        }

        const std::vector<opendeck::common::dfu::flash_area::Sector>& erase_calls() const
        {
            return _area.erase_calls();
        }

        void set_write_block_size(size_t size)
        {
            _area.set_write_block_size(size);
        }

        void set_write_result(bool result)
        {
            _area.set_write_result(result);
        }

        uint32_t size() const
        {
            return _area.size();
        }

        private:
        opendeck::common::dfu::flash_area::HwaTest _area;
    };
}    // namespace opendeck::firmware::dfu::staged_update_writer
