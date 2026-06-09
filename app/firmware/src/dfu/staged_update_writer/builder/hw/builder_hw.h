/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/flash_area/hwa/hw/hwa_hw.h"
#include "firmware/src/dfu/staged_update_writer/hwa/hw/hwa_hw.h"
#include "firmware/src/dfu/staged_update_writer/instance/impl/staged_update_writer.h"

#include <zephyr/devicetree.h>
#include <zephyr/devicetree/partitions.h>

#define OPENDECK_STAGED_DFU_NODE DT_NODELABEL(staged_dfu_partition)

namespace opendeck::firmware::dfu::staged_update_writer
{
    /**
     * @brief Builder that wires staged-update storage to real flash.
     */
    class Builder
    {
        public:
        Builder()
            : _flash_area(DT_PARTITION_ID(OPENDECK_STAGED_DFU_NODE))
            , _hwa(_flash_area)
            , _instance(_hwa)
        {}

        StagedUpdateWriter& instance()
        {
            return _instance;
        }

        private:
        opendeck::common::dfu::flash_area::HwaHw _flash_area;
        HwaHw                                    _hwa;
        StagedUpdateWriter                       _instance;
    };
}    // namespace opendeck::firmware::dfu::staged_update_writer
