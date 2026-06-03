/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/dfu/staged_update_writer/instance/impl/deps.h"
#include "common/src/dfu/flash_area/hwa/hw/hwa_hw.h"

#include <zephyr/devicetree.h>
#include <zephyr/devicetree/partitions.h>

#define OPENDECK_STAGED_DFU_NODE DT_NODELABEL(staged_dfu_partition)

namespace opendeck::firmware::dfu::staged_update_writer
{
    /**
     * @brief Zephyr flash-map backend for staged DFU storage.
     */
    class HwaHw : public opendeck::common::dfu::flash_area::HwaHw
    {
        public:
        /**
         * @brief Opens the staged DFU flash area.
         */
        bool open([[maybe_unused]] uint8_t area_id) override
        {
            return opendeck::common::dfu::flash_area::HwaHw::open(AREA_ID);
        }

        private:
        static constexpr uint8_t AREA_ID = DT_PARTITION_ID(OPENDECK_STAGED_DFU_NODE);
    };
}    // namespace opendeck::firmware::dfu::staged_update_writer
