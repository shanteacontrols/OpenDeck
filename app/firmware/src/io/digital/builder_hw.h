/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "digital.h"
#include "frame_store.h"
#include "drivers/driver.h"
#include "io/digital/buttons/buttons.h"
#include "io/digital/buttons/filter_hw.h"
#include "io/digital/buttons/hwa_hw.h"
#include "io/digital/encoders/encoders.h"
#include "io/digital/encoders/filter_hw.h"
#include "io/digital/encoders/hwa_hw.h"
#include "database/builder.h"

namespace opendeck::io::digital
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
            : _buttons_database(database)
            , _encoders_database(database)
            , _frame_store(_driver)
            , _buttons_hwa(_frame_store)
            , _buttons(_buttons_hwa, _buttons_filter, _buttons_database)
            , _encoders_hwa(_frame_store)
            , _encoders(_encoders_hwa, _encoders_filter, _encoders_database)
            , _instance(_driver, _frame_store, _buttons, _encoders)
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
        io::buttons::Database  _buttons_database;
        io::encoders::Database _encoders_database;
        drivers::Driver        _driver;
        FrameStore             _frame_store;
        io::buttons::HwaHw     _buttons_hwa;
        io::buttons::FilterHw  _buttons_filter;
        io::buttons::Buttons   _buttons;
        io::encoders::HwaHw    _encoders_hwa;
        io::encoders::FilterHw _encoders_filter;
        io::encoders::Encoders _encoders;
        Digital                _instance;
    };
}    // namespace opendeck::io::digital
