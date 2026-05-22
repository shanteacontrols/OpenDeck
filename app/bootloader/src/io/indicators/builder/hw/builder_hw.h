/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/io/indicators/hwa/hw/hwa_hw.h"
#include "bootloader/src/io/indicators/instance/impl/indicators.h"

namespace opendeck::bootloader::io::indicators
{
    /**
     * @brief Convenience builder that wires hardware bootloader indicators.
     */
    class Builder
    {
        public:
        Builder()
            : _instance(_hwa)
        {}

        /**
         * @brief Returns the bootloader indicators instance.
         *
         * @return Hardware-backed indicators instance.
         */
        Indicators& instance()
        {
            return _instance;
        }

        private:
        HwaHw      _hwa;
        Indicators _instance;
    };
}    // namespace opendeck::bootloader::io::indicators
