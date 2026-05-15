/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/peripherals/display/display.h"
#include "firmware/src/database/database.h"

namespace opendeck::io::i2c
{
    /**
     * @brief Builder that instantiates the I2C peripheral set.
     */
    class BuilderPeripherals
    {
        public:
        /**
         * @brief Constructs the peripheral bundle around the shared I2C backend and database.
         *
         * @param hwa I2C backend used by peripherals.
         * @param database Database administrator used for peripheral configuration views.
         */
        BuilderPeripherals(Hwa& hwa, database::Admin& database)
            : _display_database(database)
            , _display(hwa, _display_database)
        {}

        private:
        display::Database _display_database;
        display::Display  _display;
    };
}    // namespace opendeck::io::i2c
