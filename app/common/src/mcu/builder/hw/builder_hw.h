/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/mcu/hwa/hw/hwa_hw.h"

namespace opendeck::common::mcu
{
    /**
     * @brief Hardware MCU builder.
     */
    class Builder
    {
        public:
        Builder() = default;

        /**
         * @brief Returns the configured hardware MCU services.
         *
         * @return MCU service backend.
         */
        opendeck::common::mcu::Hwa& instance()
        {
            return _hwa;
        }

        private:
        opendeck::common::mcu::HwaHw _hwa;
    };
}    // namespace opendeck::common::mcu
