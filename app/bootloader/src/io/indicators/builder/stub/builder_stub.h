/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/io/indicators/instance/stub/indicators_stub.h"

namespace opendeck::bootloader::io::indicators
{
    /**
     * @brief Convenience builder that wires no-op bootloader indicators.
     */
    class Builder
    {
        public:
        /**
         * @brief Returns the no-op bootloader indicators instance.
         *
         * @return No-op indicators instance.
         */
        IndicatorsStub& instance()
        {
            return _instance;
        }

        private:
        IndicatorsStub _instance;
    };
}    // namespace opendeck::bootloader::io::indicators
