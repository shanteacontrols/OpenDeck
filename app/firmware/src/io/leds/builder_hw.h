/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "leds.h"
#include "hwa_hw.h"
#include "drivers/driver.h"
#include "database/builder.h"

namespace io::leds
{
    /**
     * @brief Convenience builder that wires the hardware LED subsystem.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the LED builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        explicit Builder(database::Admin& database)
            : _database(database)
            , _hwa(_driver)
            , _instance(_hwa, _database)
        {}

        /**
         * @brief Returns the constructed LED subsystem instance.
         *
         * @return Hardware-backed LED subsystem instance.
         */
        Leds& instance()
        {
            return _instance;
        }

        private:
        Database _database;
        Driver   _driver;
        HwaHw    _hwa;
        Leds     _instance;
    };
}    // namespace io::leds
