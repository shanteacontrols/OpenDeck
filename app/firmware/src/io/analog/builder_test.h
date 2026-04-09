/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "analog.h"
#include "filter_test.h"
#include "hwa_test.h"
#include "database/builder_test.h"

namespace io::analog
{
    /**
     * @brief Test builder that wires the analog subsystem to test backends.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the analog test builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        Builder(database::Admin& database)
            : _database(database)
            , _instance(_hwa, _filter, _database)
        {}

        /**
         * @brief Returns the constructed analog subsystem instance.
         *
         * @return Test-backed analog subsystem instance.
         */
        Analog& instance()
        {
            return _instance;
        }

        Database   _database;
        HwaTest    _hwa;
        FilterTest _filter;
        Analog     _instance;
    };
}    // namespace io::analog
