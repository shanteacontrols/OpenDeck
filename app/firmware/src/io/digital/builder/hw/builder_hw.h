/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/instance/impl/digital.h"
#include "firmware/src/io/digital/instance/impl/frame_store.h"
#include "firmware/src/io/digital/drivers/driver.h"
#include "firmware/src/io/digital/hwa/hw/hwa_hw.h"
#include "firmware/src/io/digital/switches/instance/impl/switches.h"
#include "firmware/src/io/digital/switches/filter/hw/filter_hw.h"
#include "firmware/src/io/digital/switches/hwa/hw/hwa_hw.h"
#include "firmware/src/io/digital/encoders/instance/impl/encoders.h"
#include "firmware/src/io/digital/encoders/filter/hw/filter_hw.h"
#include "firmware/src/io/digital/encoders/hwa/hw/hwa_hw.h"
#include "firmware/src/database/builder/builder.h"

namespace opendeck::firmware::io::digital
{
    /**
     * @brief Convenience builder that wires the hardware digital subsystem.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the digital builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        explicit Builder(database::Admin& database)
            : _switches_database(database)
            , _encoders_database(database)
            , _hwa(_driver)
            , _frame_store(_hwa)
            , _switches_hwa(_frame_store)
            , _switches(_switches_hwa, _switches_filter, _switches_database)
            , _encoders_hwa(_frame_store)
            , _encoders(_encoders_hwa, _encoders_filter, _encoders_database)
            , _instance(_hwa, _frame_store, _switches, _encoders)
        {}

        /**
         * @brief Returns the constructed digital subsystem instance.
         *
         * @return Hardware-backed digital subsystem instance.
         */
        Digital& instance()
        {
            return _instance;
        }

        private:
        io::switches::Database _switches_database;
        io::encoders::Database _encoders_database;
        drivers::Driver        _driver;
        HwaHw                  _hwa;
        FrameStore             _frame_store;
        io::switches::HwaHw    _switches_hwa;
        io::switches::FilterHw _switches_filter;
        io::switches::Switches _switches;
        io::encoders::HwaHw    _encoders_hwa;
        io::encoders::FilterHw _encoders_filter;
        io::encoders::Encoders _encoders;
        Digital                _instance;
    };
}    // namespace opendeck::firmware::io::digital
