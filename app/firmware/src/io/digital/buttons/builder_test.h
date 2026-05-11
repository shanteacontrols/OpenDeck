/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "buttons.h"
#include "filter_test.h"
#include "hwa_test.h"
#include "mapper.h"
#include "database/builder_test.h"

namespace opendeck::io::buttons
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
            , _mapper(_database)
            , _instance(_hwa, _filter, _mapper, _database)
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

        Database   _database;
        HwaTest    _hwa;
        FilterTest _filter;
        Mapper     _mapper;
        Buttons    _instance;
    };
}    // namespace opendeck::io::buttons
