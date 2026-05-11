/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "osc.h"
#include "hwa_hw.h"
#include "database/builder.h"

namespace opendeck::protocol::osc
{
    /**
     * @brief Convenience builder that wires the OSC subsystem.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the OSC builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        explicit Builder(database::Admin& database)
            : _database(database)
            , _instance(_hwa, _database)
        {}

        /**
         * @brief Returns the constructed OSC backend.
         *
         * @return Hardware-backed OSC protocol instance.
         */
        Osc& instance()
        {
            return _instance;
        }

        private:
        HwaHw    _hwa;
        Database _database;
        Osc      _instance;
    };
}    // namespace opendeck::protocol::osc
