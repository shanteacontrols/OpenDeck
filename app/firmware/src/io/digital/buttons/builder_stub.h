/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "buttons_stub.h"
#include "filter_stub.h"
#include "hwa_stub.h"
#include "database/builder.h"

namespace opendeck::io::buttons
{
    /**
     * @brief Stub builder that wires the button subsystem to no-op backends.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the button stub builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        explicit Builder(database::Admin& database)
            : _database(database)
            , _instance(_hwa, _filter, _database)
        {}

        /**
         * @brief Returns the constructed button subsystem instance.
         *
         * @return Stub-backed button subsystem instance.
         */
        Buttons& instance()
        {
            return _instance;
        }

        private:
        HwaStub    _hwa;
        FilterTest _filter;
        Database   _database;
        Buttons    _instance;
    };
}    // namespace opendeck::io::buttons
