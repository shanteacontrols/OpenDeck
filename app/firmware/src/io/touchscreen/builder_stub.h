/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/touchscreen/touchscreen_stub.h"
#include "firmware/src/io/touchscreen/hwa_stub.h"
#include "firmware/src/database/builder.h"

namespace opendeck::io::touchscreen
{
    /**
     * @brief Stub builder that wires the touchscreen subsystem to a no-op backend.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the touchscreen stub builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        explicit Builder(database::Admin& database)
            : _database(database)
            , _instance(_hwa, _database)
        {}

        /**
         * @brief Returns the constructed touchscreen subsystem instance.
         *
         * @return Stub-backed touchscreen subsystem instance.
         */
        Touchscreen& instance()
        {
            return _instance;
        }

        private:
        HwaStub     _hwa;
        Database    _database;
        Touchscreen _instance;
    };
}    // namespace opendeck::io::touchscreen
