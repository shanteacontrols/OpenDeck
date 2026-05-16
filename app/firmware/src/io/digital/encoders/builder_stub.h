/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/encoders/encoders_stub.h"
#include "firmware/src/io/digital/encoders/filter_stub.h"
#include "firmware/src/io/digital/encoders/hwa/stub/hwa_stub.h"
#include "firmware/src/io/digital/switches/deps.h"
#include "firmware/src/database/builder.h"

namespace opendeck::io::encoders
{
    /**
     * @brief Stub builder that wires the encoder subsystem to no-op backends.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the encoder stub builder.
         *
         * @param database Database administrator used for configuration access.
         */
        explicit Builder(database::Admin& database)
            : _database(database)
            , _instance(_hwa, _filter, _database)
        {}

        /**
         * @brief Constructs the encoder stub builder while accepting an unused switch HWA.
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
         * @return Stub-backed encoder subsystem instance.
         */
        Encoders& instance()
        {
            return _instance;
        }

        private:
        HwaStub    _hwa;
        FilterStub _filter;
        Database   _database;
        Encoders   _instance;
    };
}    // namespace opendeck::io::encoders
