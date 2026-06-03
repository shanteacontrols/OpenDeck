/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/dfu/staged_update_writer/instance/impl/deps.h"
#include "common/src/dfu/flash_area/hwa/hw/hwa_hw.h"

#include <zephyr/devicetree.h>
#include <zephyr/devicetree/partitions.h>

#include <optional>

namespace opendeck::firmware::dfu::staged_update_writer
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
            if (!_initialized)
            {
                return std::nullopt;
            }

            const auto sector = _flash_area.sector(index);

            if (!sector)
            {
                return std::nullopt;
            }

            return Sector{
                .offset = sector->offset,
                .size   = sector->size,
            };
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

        opendeck::common::dfu::flash_area::HwaHw _flash_area  = {};
        bool                                     _initialized = false;
    };
}    // namespace opendeck::firmware::dfu::staged_update_writer
