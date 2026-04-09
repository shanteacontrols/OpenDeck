/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "buttons.h"
#include "filter_test.h"
#include "hwa_test.h"
#include "database/builder_test.h"

namespace io::buttons
{
    /**
     * @brief Test builder that wires the button subsystem to test backends.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the button test builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        Builder(database::Admin& database)
            : _database(database)
            , _instance(_hwa, _filter, _database)
        {}

        /**
         * @brief Returns the constructed button subsystem instance.
         *
         * @return Test-backed button subsystem instance.
         */
        Buttons& instance()
        {
            return _instance;
        }

        HwaTest    _hwa;
        FilterTest _filter;
        Database   _database;
        Buttons    _instance;
    };
}    // namespace io::buttons
