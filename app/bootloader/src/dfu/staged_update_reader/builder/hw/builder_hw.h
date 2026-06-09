/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/staged_update_reader/hwa/hw/hwa_hw.h"
#include "bootloader/src/dfu/staged_update_reader/instance/impl/staged_update_reader.h"
#include "common/src/dfu/flash_area/hwa/hw/hwa_hw.h"

#include <zephyr/devicetree.h>
#include <zephyr/devicetree/partitions.h>

#define OPENDECK_STAGED_DFU_NODE DT_NODELABEL(staged_dfu_partition)

namespace opendeck::bootloader::dfu::staged_update_reader
{
    /**
     * @brief Builder that wires staged-update reader storage to real flash.
     */
    class Builder
    {
        public:
        Builder()
            : _flash_area(DT_PARTITION_ID(OPENDECK_STAGED_DFU_NODE))
            , _hwa(_flash_area)
            , _instance(_hwa)
        {}

        StagedUpdateReader& instance()
        {
            return _instance;
        }

        private:
        opendeck::common::dfu::flash_area::HwaHw _flash_area;
        HwaHw                                    _hwa;
        StagedUpdateReader                       _instance;
    };
}    // namespace opendeck::bootloader::dfu::staged_update_reader
