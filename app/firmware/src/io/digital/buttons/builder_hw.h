/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "buttons.h"
#include "filter_hw.h"
#include "hwa_hw.h"
#include "drivers/driver.h"
#include "database/builder.h"

namespace opendeck::io::buttons
{
    /**
     * @brief Convenience builder that wires the hardware button subsystem.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the button builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        Builder(database::Admin& database)
            : _database(database)
            , _hwa(_driver)
            , _instance(_hwa, _filter, _database)
        {}

        /**
         * @brief Returns the constructed button subsystem instance.
         *
         * @return Hardware-backed button subsystem instance.
         */
        Buttons& instance()
        {
            return _instance;
        }

        /**
         * @brief Returns the button hardware backend used by the builder.
         *
         * @return Button HWA instance.
         */
        Hwa& hwa()
        {
            return _hwa;
        }

        private:
        Database _database;
        Driver   _driver;
        HwaHw    _hwa;
        FilterHw _filter;
        Buttons  _instance;
    };
}    // namespace opendeck::io::buttons
