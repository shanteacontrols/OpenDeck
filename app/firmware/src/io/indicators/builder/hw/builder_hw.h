/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/indicators/indicators.h"
#include "firmware/src/io/indicators/hwa/hw/hwa_hw.h"

namespace opendeck::io::indicators
{
    /**
     * @brief Convenience builder that wires the hardware indicator subsystem.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the indicator builder.
         *
         * @param database Unused database handle kept for builder-interface consistency.
         */
        explicit Builder([[maybe_unused]] database::Admin& database)
            : _instance(_hwa)
        {}

        /**
         * @brief Returns the constructed indicator subsystem instance.
         *
         * @return Hardware-backed indicator subsystem instance.
         */
        Indicators& instance()
        {
            return _instance;
        }

        private:
        HwaHw      _hwa;
        Indicators _instance;
    };
}    // namespace opendeck::io::indicators
