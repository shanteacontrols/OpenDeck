/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "analog.h"
#include "drivers/driver.h"
#include "filter_hw.h"
#include "hwa_hw.h"
#include "database/builder.h"

namespace opendeck::io::analog
{
    /**
     * @brief Convenience builder that wires the hardware analog subsystem.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the analog builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        explicit Builder(database::Admin& database)
            : _database(database)
            , _hwa(_driver)
            , _instance(_hwa, _filter, _database)
        {}

        /**
         * @brief Returns the constructed analog subsystem instance.
         *
         * @return Hardware-backed analog subsystem instance.
         */
        Analog& instance()
        {
            return _instance;
        }

        private:
        Database                           _database;
        Driver                             _driver;
        HwaHw                              _hwa;
        FilterHw<OPENDECK_ANALOG_ADC_BITS> _filter;
        Analog                             _instance;
    };
}    // namespace opendeck::io::analog
