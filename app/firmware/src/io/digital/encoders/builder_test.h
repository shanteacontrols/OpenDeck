/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/encoders/encoders.h"
#include "firmware/src/io/digital/encoders/filter_test.h"
#include "firmware/src/io/digital/encoders/hwa/test/hwa_test.h"
#include "firmware/src/io/digital/switches/deps.h"
#include "firmware/src/database/builder_test.h"

namespace opendeck::io::encoders
{
    /**
     * @brief Test builder that wires the encoder subsystem to test backends.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the encoder test builder.
         *
         * @param database Database administrator used for configuration access.
         */
        Builder(database::Admin& database)
            : _database(database)
            , _mapper(_database)
            , _instance(_hwa, _filter, _database, _mapper)
        {}

        /**
         * @brief Constructs the encoder test builder while accepting an unused switch HWA.
         *
         * @param database Database administrator used for configuration access.
         * @param switches_hwa Unused switch HWA kept for builder-interface compatibility.
         */
        Builder(database::Admin&                    database,
                [[maybe_unused]] io::switches::Hwa& switches_hwa)
            : Builder(database)
        {}

        /**
         * @brief Returns the constructed encoder subsystem instance.
         *
         * @return Test-backed encoder subsystem instance.
         */
        Encoders& instance()
        {
            return _instance;
        }

        HwaTest    _hwa;
        FilterTest _filter;
        Database   _database;
        Mapper     _mapper;
        Encoders   _instance;
    };
}    // namespace opendeck::io::encoders
