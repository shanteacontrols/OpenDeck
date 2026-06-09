/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/flash_area/shared/deps.h"
#include "common/src/dfu/staged_update/shared/deps.h"

namespace opendeck::common::dfu::staged_update
{
    /**
     * @brief Backend for staged DFU storage backed by a configured flash area.
     */
    class HwaHw : public Hwa
    {
        public:
        /**
         * @brief Constructs staged DFU storage around a flash area.
         *
         * @param area Configured staged DFU flash area.
         */
        explicit HwaHw(opendeck::common::dfu::flash_area::Hwa& area)
            : _area(area)
        {}

        opendeck::common::dfu::flash_area::Hwa& flash_area() override
        {
            return _area;
        }

        private:
        opendeck::common::dfu::flash_area::Hwa& _area;
    };
}    // namespace opendeck::common::dfu::staged_update
