/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/instance/impl/outputs.h"
#include "firmware/src/io/outputs/hwa/hw/hwa_hw.h"
#include "firmware/src/io/outputs/drivers/driver.h"
#include "firmware/src/database/builder/builder.h"

namespace opendeck::firmware::io::outputs
{
    /**
     * @brief Convenience builder that wires the hardware output subsystem.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the output builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        explicit Builder(database::Admin& database)
            : _database(database)
            , _hwa(_driver)
            , _instance(_hwa, _database)
        {}

        /**
         * @brief Returns the constructed output subsystem instance.
         *
         * @return Hardware-backed output subsystem instance.
         */
        Outputs& instance()
        {
            return _instance;
        }

        private:
        Database _database;
        Driver   _driver;
        HwaHw    _hwa;
        Outputs  _instance;
    };
}    // namespace opendeck::firmware::io::outputs
