/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "switches_stub.h"
#include "filter_stub.h"
#include "hwa_stub.h"
#include "database/builder.h"

namespace opendeck::io::switches
{
    /**
     * @brief Stub builder that wires the switch subsystem to no-op backends.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the switch stub builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        explicit Builder(database::Admin& database)
            : _database(database)
            , _instance(_hwa, _filter, _database)
        {}

        /**
         * @brief Returns the constructed switch subsystem instance.
         *
         * @return Stub-backed switch subsystem instance.
         */
        Switches& instance()
        {
            return _instance;
        }

        private:
        HwaStub    _hwa;
        FilterTest _filter;
        Database   _database;
        Switches   _instance;
    };
}    // namespace opendeck::io::switches
