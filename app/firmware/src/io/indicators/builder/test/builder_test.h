/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/indicators/instance/impl/indicators.h"
#include "common/src/indicators/hwa/test/hwa_test.h"

namespace opendeck::io::indicators
{
    /**
     * @brief Test builder that wires the indicator subsystem to a test backend.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the test indicator builder.
         *
         * @param database Unused database handle kept for builder-interface consistency.
         */
        Builder(database::Admin& database)
            : _instance(_hwa)
        {}

        /**
         * @brief Returns the constructed indicator subsystem instance.
         *
         * @return Test-backed indicator subsystem instance.
         */
        Indicators& instance()
        {
            return _instance;
        }

        private:
        HwaTest    _hwa;
        Indicators _instance;
    };
}    // namespace opendeck::io::indicators
