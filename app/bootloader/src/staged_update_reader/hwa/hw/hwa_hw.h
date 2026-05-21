/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/indicators/instance/impl/indicators.h"
#include "bootloader/src/staged_update_reader/shared/deps.h"
#include "common/src/flash_area/hwa/hw/hwa_hw.h"

#include <zephyr/devicetree.h>
#include <zephyr/devicetree/fixed-partitions.h>

#include <span>

#define OPENDECK_STAGED_DFU_NODE DT_NODELABEL(staged_dfu_partition)

namespace opendeck::staged_update_reader
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
         * @brief Clears the staged-update marker without erasing the whole partition.
         */
        void clear_pending() override
        {
            const uint32_t invalid_magic = 0U;
            const auto     data          = std::span<const uint8_t>(
                reinterpret_cast<const uint8_t*>(&invalid_magic),
                sizeof(invalid_magic));

            _area.write(0, data);
        }

        /**
         * @brief Initializes bootloader indicators before staged update processing.
         */
        void on_update_start() override
        {
            indicators::init();
        }

        private:
        static constexpr uint8_t  STAGED_DFU_AREA_ID        = DT_PARTITION_ID(OPENDECK_STAGED_DFU_NODE);
        static constexpr uint32_t STAGED_DFU_PARTITION_SIZE = DT_REG_SIZE(OPENDECK_STAGED_DFU_NODE);

        flash_area::HwaHw _area;
    };
}    // namespace opendeck::staged_update_reader
