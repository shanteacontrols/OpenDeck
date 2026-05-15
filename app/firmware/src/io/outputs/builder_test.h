/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/outputs.h"
#include "firmware/src/io/outputs/hwa_test.h"
#include "firmware/src/database/builder_test.h"

namespace opendeck::io::outputs
{
    /**
     * @brief Test builder that wires the OUTPUT subsystem to a test backend.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the OUTPUT test builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        Builder(database::Admin& database)
            : _database(database)
            , _instance(_hwa, _database)
        {}

        /**
         * @brief Returns the constructed OUTPUT subsystem instance.
         *
         * @return Test-backed OUTPUT subsystem instance.
         */
        Outputs& instance()
        {
            return _instance;
        }

        HwaTest  _hwa;
        Database _database;
        Outputs  _instance;
    };
}    // namespace opendeck::io::outputs
