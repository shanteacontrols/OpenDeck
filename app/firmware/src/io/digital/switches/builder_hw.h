/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/switches/switches.h"
#include "firmware/src/io/digital/switches/filter_hw.h"
#include "firmware/src/io/digital/switches/hwa/hw/hwa_hw.h"
#include "firmware/src/io/digital/switches/mapper.h"
#include "firmware/src/io/digital/drivers/driver.h"
#include "firmware/src/database/builder.h"

namespace opendeck::io::switches
{
    /**
     * @brief Convenience builder that wires the hardware switch subsystem.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the switch builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        Builder(database::Admin& database)
            : _database(database)
            , _hwa(_driver)
            , _mapper(_database)
            , _instance(_hwa, _filter, _mapper, _database)
        {}

        /**
         * @brief Returns the constructed switch subsystem instance.
         *
         * @return Hardware-backed switch subsystem instance.
         */
        Switches& instance()
        {
            return _instance;
        }

        /**
         * @brief Returns the switch hardware backend used by the builder.
         *
         * @return Switch HWA instance.
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
        Mapper   _mapper;
        Switches _instance;
    };
}    // namespace opendeck::io::switches
