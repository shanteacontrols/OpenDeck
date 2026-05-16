/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/touchscreen/instance/impl/touchscreen.h"
#include "firmware/src/io/touchscreen/hwa/test/hwa_test.h"
#include "firmware/src/database/builder/test/builder_test.h"

namespace opendeck::io::touchscreen
{
    /**
     * @brief Test builder that wires the touchscreen subsystem to a test backend.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the touchscreen test builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        Builder(database::Admin& database)
            : _database(database)
            , _instance(_hwa, _database)
        {}

        /**
         * @brief Returns the constructed touchscreen subsystem instance.
         *
         * @return Test-backed touchscreen subsystem instance.
         */
        Touchscreen& instance()
        {
            return _instance;
        }

        HwaTest     _hwa;
        Database    _database;
        Touchscreen _instance;
    };
}    // namespace opendeck::io::touchscreen
