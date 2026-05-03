/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "indicators.h"
#include "hwa_stub.h"

namespace opendeck::io::indicators
{
    /**
     * @brief Stub builder that wires the indicator subsystem to a no-op backend.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the stub indicator builder.
         *
         * @param database Unused database handle kept for builder-interface consistency.
         */
        explicit Builder([[maybe_unused]] database::Admin& database)
            : _instance(_hwa)
        {}

        /**
         * @brief Returns the constructed indicator subsystem instance.
         *
         * @return Stub-backed indicator subsystem instance.
         */
        Indicators& instance()
        {
            return _instance;
        }

        private:
        HwaStub    _hwa;
        Indicators _instance;
    };
}    // namespace opendeck::io::indicators
