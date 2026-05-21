/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/staged_update_writer/shared/deps.h"
#include "common/src/flash_area/hwa/hw/hwa_hw.h"

#include <zephyr/devicetree.h>
#include <zephyr/devicetree/partitions.h>

#include <optional>

namespace opendeck::staged_update_writer
{
    /**
     * @brief Zephyr flash-map backend for staged DFU storage.
     */
    class HwaHw : public Hwa
    {
        public:
        bool init() override
        {
            if (_initialized)
            {
                return true;
            }

            if (!_flash_area.open(STAGED_DFU_AREA_ID))
            {
                return false;
            }

            flash_area::Hwa::Sector first_sector = {};

            if (!_flash_area.first_sector(first_sector))
            {
                return false;
            }

            _first_sector = Sector{
                .offset = first_sector.offset,
                .size   = first_sector.size,
            };

            _initialized = true;
            return true;
        }

        uint32_t size() const override
        {
            return STAGED_DFU_PARTITION_SIZE;
        }

        size_t write_block_size() const override
        {
            return STAGED_DFU_WRITE_BLOCK_SIZE;
        }

        std::optional<Sector> sector(size_t index) const override
        {
            if (index != 0U)
            {
                return std::nullopt;
            }

            return _first_sector;
        }

        bool erase(uint32_t offset, uint32_t size) override
        {
            return _flash_area.erase(offset, size);
        }

        bool write(uint32_t offset, std::span<const uint8_t> data) override
        {
            return _flash_area.write(offset, data);
        }

        private:
#define OPENDECK_STAGED_DFU_NODE DT_NODELABEL(staged_dfu_partition)
        static constexpr uint8_t  STAGED_DFU_AREA_ID          = DT_PARTITION_ID(OPENDECK_STAGED_DFU_NODE);
        static constexpr size_t   STAGED_DFU_WRITE_BLOCK_SIZE = DT_PROP(DT_MEM_FROM_PARTITION(OPENDECK_STAGED_DFU_NODE), write_block_size);
        static constexpr uint32_t STAGED_DFU_PARTITION_SIZE   = DT_REG_SIZE(OPENDECK_STAGED_DFU_NODE);

        flash_area::HwaHw _flash_area   = {};
        Sector            _first_sector = {};
        bool              _initialized  = false;
    };
}    // namespace opendeck::staged_update_writer
