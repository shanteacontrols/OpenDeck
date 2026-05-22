/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/io/indicators/hwa/test/hwa_test.h"
#include "bootloader/src/io/indicators/instance/impl/indicators.h"

namespace opendeck::bootloader::io::indicators
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
}    // namespace opendeck::bootloader::io::indicators
