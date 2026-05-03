/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "leds.h"
#include "hwa_test.h"
#include "database/builder_test.h"

namespace opendeck::io::leds
{
    /**
     * @brief Test builder that wires the LED subsystem to a test backend.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the LED test builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        Builder(database::Admin& database)
            : _database(database)
            , _instance(_hwa, _database)
        {}

        /**
         * @brief Returns the constructed LED subsystem instance.
         *
         * @return Test-backed LED subsystem instance.
         */
        Leds& instance()
        {
            return _instance;
        }

        HwaTest  _hwa;
        Database _database;
        Leds     _instance;
    };
}    // namespace opendeck::io::leds
