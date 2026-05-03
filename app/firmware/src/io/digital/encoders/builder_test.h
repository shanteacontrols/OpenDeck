/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "encoders.h"
#include "filter_test.h"
#include "hwa_test.h"
#include "io/digital/buttons/deps.h"
#include "database/builder_test.h"

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
            , _instance(_hwa, _filter, _database)
        {}

        /**
         * @brief Constructs the encoder test builder while accepting an unused button HWA.
         *
         * @param database Database administrator used for configuration access.
         * @param buttons_hwa Unused button HWA kept for builder-interface compatibility.
         */
        Builder(database::Admin&                   database,
                [[maybe_unused]] io::buttons::Hwa& buttons_hwa)
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
        Encoders   _instance;
    };
}    // namespace opendeck::io::encoders
