/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/staged_update/shared/deps.h"

namespace opendeck::bootloader::dfu::staged_update_reader
{
    /**
     * @brief Hardware abstraction used to access staged firmware payload storage.
     */
    class Hwa : public opendeck::common::dfu::staged_update::Hwa
    {
        public:
        /**
         * @brief Invalidates the staged-update marker.
         *
         * @return `true` when the marker was invalidated.
         */
        virtual bool clear_pending() = 0;
    };
}    // namespace opendeck::bootloader::dfu::staged_update_reader
