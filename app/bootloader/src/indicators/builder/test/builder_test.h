/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/indicators/hwa/test/hwa_test.h"
#include "bootloader/src/indicators/instance/impl/indicators.h"

namespace opendeck::bootloader::indicators
{
    /**
     * @brief Convenience builder that wires test bootloader indicators.
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
         * @return Test-backed indicators instance.
         */
        Indicators& instance()
        {
            return _instance;
        }

        HwaTest _hwa;

        private:
        Indicators _instance;
    };
}    // namespace opendeck::bootloader::indicators
