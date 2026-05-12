/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "outputs_stub.h"
#include "hwa_stub.h"
#include "database/builder.h"

namespace opendeck::io::outputs
{
    /**
     * @brief Stub builder that wires the OUTPUT subsystem to a no-op backend.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the OUTPUT stub builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        explicit Builder(database::Admin& database)
            : _database(database)
            , _instance(_hwa, _database)
        {}

        /**
         * @brief Returns the constructed OUTPUT subsystem instance.
         *
         * @return Stub-backed OUTPUT subsystem instance.
         */
        Outputs& instance()
        {
            return _instance;
        }

        private:
        HwaStub  _hwa;
        Database _database;
        Outputs  _instance;
    };
}    // namespace opendeck::io::outputs
