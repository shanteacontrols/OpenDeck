/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/analog/analog.h"
#include "firmware/src/io/analog/filter_test.h"
#include "firmware/src/io/analog/hwa_test.h"
#include "firmware/src/io/analog/mapper.h"
#include "firmware/src/database/builder_test.h"

namespace opendeck::io::analog
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
            , _mapper(_database)
            , _instance(_hwa, _filter, _mapper, _database)
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
        Mapper     _mapper;
        Analog     _instance;
    };
}    // namespace opendeck::io::analog
