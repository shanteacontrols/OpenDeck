/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/instance/stub/outputs_stub.h"
#include "firmware/src/io/outputs/hwa/stub/hwa_stub.h"
#include "firmware/src/database/builder/builder.h"

namespace opendeck::firmware::io::outputs
{
    /**
     * @brief Stub builder that wires the output subsystem to a no-op backend.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the output stub builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        explicit Builder(database::Admin& database)
            : _database(database)
            , _instance(_hwa, _database)
        {}

        /**
         * @brief Returns the constructed output subsystem instance.
         *
         * @return Stub-backed output subsystem instance.
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
}    // namespace opendeck::firmware::io::outputs
