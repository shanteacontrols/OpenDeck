/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/database/instance/impl/database.h"
#include "firmware/src/io/indicators/instance/stub/indicators_stub.h"

namespace opendeck::io::indicators
{
    /**
     * @brief Stub builder that exposes a no-op indicator subsystem.
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
        Indicators _instance;
    };
}    // namespace opendeck::io::indicators
