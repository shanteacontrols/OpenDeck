/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/staged_update_reader/instance/impl/deps.h"
#include "common/src/dfu/staged_update/hwa/hw/hwa_hw.h"

namespace opendeck::bootloader::dfu::staged_update_reader
{
    /**
     * @brief Hardware-backed staged-update reader storage.
     */
    class HwaHw : public Hwa
    {
        public:
        /**
         * @brief Constructs staged reader storage around a staged DFU backend.
         */
        explicit HwaHw(opendeck::common::dfu::flash_area::Hwa& area)
            : _hwa(area)
        {}

        /**
         * @brief Returns the staged DFU flash area.
         */
        opendeck::common::dfu::flash_area::Hwa& flash_area() override
        {
            return _hwa.flash_area();
        }

        /**
         * @brief Clears the staged-update marker by erasing the header sector.
         *
         * @return `true` when the header sector was erased, otherwise `false`.
         */
        bool clear_pending() override
        {
            auto&      area          = _hwa.flash_area();
            const auto header_sector = area.sector(0);

            return header_sector.has_value() &&
                   area.erase(header_sector->offset, header_sector->size);
        }

        private:
        opendeck::common::dfu::staged_update::HwaHw _hwa;
    };
}    // namespace opendeck::bootloader::dfu::staged_update_reader
