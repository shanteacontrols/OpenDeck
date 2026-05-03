/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "leds_stub.h"
#include "hwa_stub.h"
#include "database/builder.h"

namespace opendeck::io::leds
{
    /**
     * @brief Stub builder that wires the LED subsystem to a no-op backend.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the LED stub builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        explicit Builder(database::Admin& database)
            : _database(database)
            , _instance(_hwa, _database)
        {}

        /**
         * @brief Returns the constructed LED subsystem instance.
         *
         * @return Stub-backed LED subsystem instance.
         */
        Leds& instance()
        {
            return _instance;
        }

        private:
        HwaStub  _hwa;
        Database _database;
        Leds     _instance;
    };
}    // namespace opendeck::io::leds
