/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/staged_update/shared/deps.h"

#include <zephyr/devicetree.h>
#include <zephyr/devicetree/fixed-partitions.h>
#include <zephyr/storage/flash_map.h>

#include <array>
#include <optional>

namespace opendeck::staged_update
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

            if (flash_area_open(STAGED_DFU_AREA_ID, &_flash_area) != 0)
            {
                return false;
            }

            _sector_count = _sectors.size();

            if (flash_area_get_sectors(STAGED_DFU_AREA_ID, &_sector_count, _sectors.data()) != 0)
            {
                _sector_count = 0;
                return false;
            }

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
            if (index >= _sector_count)
            {
                return std::nullopt;
            }

            return Sector{
                .offset = static_cast<uint32_t>(_sectors[index].fs_off),
                .size   = _sectors[index].fs_size,
            };
        }

        bool erase(uint32_t offset, uint32_t size) override
        {
            return (_flash_area != nullptr) &&
                   (flash_area_erase(_flash_area, static_cast<off_t>(offset), size) == 0);
        }

        bool write(uint32_t offset, std::span<const uint8_t> data) override
        {
            return (_flash_area != nullptr) &&
                   (flash_area_write(_flash_area, static_cast<off_t>(offset), data.data(), data.size()) == 0);
        }

        private:
        static constexpr size_t MAX_SECTOR_COUNT = 128;

#define OPENDECK_BOOTLOADER_NODE DT_NODELABEL(opendeck_bootloader)

#define OPENDECK_STAGED_DFU_NODE DT_PHANDLE(OPENDECK_BOOTLOADER_NODE, staged_dfu_partition)
        static constexpr uint8_t  STAGED_DFU_AREA_ID          = DT_PARTITION_ID(OPENDECK_STAGED_DFU_NODE);
        static constexpr size_t   STAGED_DFU_WRITE_BLOCK_SIZE = DT_PROP(DT_MEM_FROM_FIXED_PARTITION(OPENDECK_STAGED_DFU_NODE), write_block_size);
        static constexpr uint32_t STAGED_DFU_PARTITION_SIZE   = DT_REG_SIZE(OPENDECK_STAGED_DFU_NODE);

        const struct flash_area*                   _flash_area   = nullptr;
        std::array<flash_sector, MAX_SECTOR_COUNT> _sectors      = {};
        size_t                                     _sector_count = 0;
        bool                                       _initialized  = false;
    };
}    // namespace opendeck::staged_update
