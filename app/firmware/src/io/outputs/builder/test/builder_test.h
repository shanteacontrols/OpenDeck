/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/instance/impl/outputs.h"
#include "firmware/src/io/outputs/hwa/test/hwa_test.h"
#include "firmware/src/database/builder/test/builder_test.h"

namespace opendeck::io::outputs
{
    /**
     * @brief Test builder that wires the output subsystem to a test backend.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the output test builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        Builder(database::Admin& database)
            : _database(database)
            , _mapper(_database)
            , _instance(_hwa, _mapper, _database)
        {}

        /**
         * @brief Returns the constructed output subsystem instance.
         *
         * @return Test-backed output subsystem instance.
         */
        Outputs& instance()
        {
            return _instance;
        }

        HwaTest  _hwa;
        Database _database;
        Mapper   _mapper;
        Outputs  _instance;
    };
}    // namespace opendeck::io::outputs
