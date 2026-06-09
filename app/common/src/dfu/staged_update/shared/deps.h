/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/flash_area/shared/deps.h"

namespace opendeck::common::dfu::staged_update
{
    /**
     * @brief Flash access used by staged-update reader and writer paths.
     */
    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        /**
         * @brief Returns the staged DFU flash area.
         */
        virtual opendeck::common::dfu::flash_area::Hwa& flash_area() = 0;
    };
}    // namespace opendeck::common::dfu::staged_update
