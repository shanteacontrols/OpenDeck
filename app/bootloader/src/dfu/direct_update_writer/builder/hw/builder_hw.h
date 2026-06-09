/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/direct_update_writer/hwa/hw/hwa_hw.h"
#include "bootloader/src/dfu/direct_update_writer/instance/impl/direct_update_writer.h"
#include "common/src/dfu/flash_area/hwa/hw/hwa_hw.h"
#include "common/src/mcu/shared/deps.h"

#include <zephyr/devicetree.h>
#include <zephyr/devicetree/partitions.h>

#define OPENDECK_DIRECT_DFU_NODE DT_NODELABEL(slot0_partition)

namespace opendeck::bootloader::dfu::direct_update_writer
{
    /**
     * @brief Convenience builder that wires the hardware direct-update writer backend.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the direct-update writer builder.
         *
         * @param mcu Shared MCU services.
         */
        explicit Builder(opendeck::common::mcu::Hwa& mcu)
            : _flash_area(DT_PARTITION_ID(OPENDECK_DIRECT_DFU_NODE))
            , _hwa(mcu)
            , _instance(_flash_area, _hwa)
        {}

        /**
         * @brief Returns the constructed direct-update writer instance.
         *
         * @return Hardware-backed direct-update writer instance.
         */
        DirectUpdateWriter& instance()
        {
            return _instance;
        }

        private:
        opendeck::common::dfu::flash_area::HwaHw _flash_area;
        HwaHw                                    _hwa;
        DirectUpdateWriter                       _instance;
    };
}    // namespace opendeck::bootloader::dfu::direct_update_writer
