/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/staged_update_reader/instance/impl/deps.h"
#include "common/src/dfu/flash_area/hwa/hw/hwa_hw.h"

#include <zephyr/devicetree.h>
#include <zephyr/devicetree/fixed-partitions.h>

#include <span>

#define OPENDECK_STAGED_DFU_NODE DT_NODELABEL(staged_dfu_partition)

namespace opendeck::bootloader::dfu::staged_update_reader
{
    /**
     * @brief Hardware-backed staged-update reader storage.
     */
    class HwaHw : public Hwa
    {
        public:
        /**
         * @brief Opens the staged DFU flash area.
         *
         * @return `true` when staged DFU storage is available.
         */
        bool init() override
        {
            return _area.open(STAGED_DFU_AREA_ID);
        }

        /**
         * @brief Returns the staged DFU partition size.
         *
         * @return Partition size in bytes.
         */
        uint32_t size() const override
        {
            return STAGED_DFU_PARTITION_SIZE;
        }

        /**
         * @brief Returns the staged DFU flash write-block size.
         *
         * @return Native write-block size in bytes.
         */
        size_t write_block_size() const override
        {
            return _area.write_block_size();
        }

        /**
         * @brief Reads bytes from the staged DFU partition.
         *
         * @param offset Byte offset within the staged DFU partition.
         * @param data Destination buffer.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool read(const uint32_t offset, std::span<uint8_t> data) override
        {
            return _area.read(offset, data);
        }

        /**
         * @brief Clears the staged-update marker by erasing the header sector.
         *
         * @return `true` when the header sector was erased, otherwise `false`.
         */
        bool clear_pending() override
        {
            const auto sector = _area.sector(0);

            return sector.has_value() &&
                   _area.erase(sector->offset, sector->size);
        }

        private:
        static constexpr uint8_t  STAGED_DFU_AREA_ID        = DT_PARTITION_ID(OPENDECK_STAGED_DFU_NODE);
        static constexpr uint32_t STAGED_DFU_PARTITION_SIZE = DT_REG_SIZE(OPENDECK_STAGED_DFU_NODE);

        opendeck::common::dfu::flash_area::HwaHw _area;
    };
}    // namespace opendeck::bootloader::dfu::staged_update_reader
