/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/switches/switches.h"
#include "firmware/src/io/digital/switches/filter_test.h"
#include "firmware/src/io/digital/switches/hwa_test.h"
#include "firmware/src/io/digital/switches/mapper.h"
#include "firmware/src/database/builder_test.h"

namespace opendeck::io::switches
{
    /**
     * @brief Test builder that wires the switch subsystem to test backends.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the switch test builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        Builder(database::Admin& database)
            : _database(database)
            , _mapper(_database)
            , _instance(_hwa, _filter, _mapper, _database)
        {}

        /**
         * @brief Returns the constructed switch subsystem instance.
         *
         * @return Test-backed switch subsystem instance.
         */
        Switches& instance()
        {
            return _instance;
        }

        Database   _database;
        HwaTest    _hwa;
        FilterTest _filter;
        Mapper     _mapper;
        Switches   _instance;
    };
}    // namespace opendeck::io::switches
