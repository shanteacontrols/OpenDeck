/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/outputs.h"
#include "firmware/src/io/outputs/hwa/hw/hwa_hw.h"
#include "firmware/src/io/outputs/drivers/driver.h"
#include "firmware/src/database/builder.h"

namespace opendeck::io::outputs
{
    /**
     * @brief Convenience builder that wires the hardware OUTPUT subsystem.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the OUTPUT builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        explicit Builder(database::Admin& database)
            : _database(database)
            , _hwa(_driver)
            , _instance(_hwa, _database)
        {}

        /**
         * @brief Returns the constructed OUTPUT subsystem instance.
         *
         * @return Hardware-backed OUTPUT subsystem instance.
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
}    // namespace opendeck::io::outputs
