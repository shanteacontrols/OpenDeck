/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/analog/analog.h"
#include "firmware/src/io/analog/drivers/driver.h"
#include "firmware/src/io/analog/filter_hw.h"
#include "firmware/src/io/analog/hwa_hw.h"
#include "firmware/src/io/analog/mapper.h"
#include "firmware/src/database/builder.h"

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
            , _mapper(_database)
            , _instance(_hwa, _filter, _mapper, _database)
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
        Database _database;
        Driver   _driver;
        HwaHw    _hwa;
        FilterHw _filter;
        Mapper   _mapper;
        Analog   _instance;
    };
}    // namespace opendeck::io::analog
