/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/osc/osc.h"
#include "firmware/src/protocol/osc/hwa/test/hwa_test.h"
#include "firmware/src/database/builder.h"

namespace opendeck::protocol::osc
{
    /**
     * @brief Test builder that wires OSC to a test backend.
     */
    class Builder
    {
        public:
        Builder()
            : _database(_default_database_builder.instance())
            , _instance(_hwa, _database)
        {}

        /**
         * @brief Constructs the OSC test builder with the same signature as the hardware builder.
         */
        explicit Builder(database::Admin& database)
            : _database(database)
            , _instance(_hwa, _database)
        {}

        /**
         * @brief Returns the constructed OSC test backend.
         *
         * @return Test-backed OSC protocol instance.
         */
        Osc& instance()
        {
            return _instance;
        }

        database::Builder _default_database_builder;
        HwaTest           _hwa;
        Database          _database;
        Osc               _instance;
    };
}    // namespace opendeck::protocol::osc
