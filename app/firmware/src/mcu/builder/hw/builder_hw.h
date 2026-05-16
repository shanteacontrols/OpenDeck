/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/mcu/hwa/hw/hwa_hw.h"

namespace opendeck::mcu
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
        Hwa& instance()
        {
            return _hwa;
        }

        private:
        HwaHw _hwa;
    };
}    // namespace opendeck::mcu
