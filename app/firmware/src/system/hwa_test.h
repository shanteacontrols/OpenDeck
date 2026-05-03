/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "database/builder.h"
#include "protocol/midi/builder.h"
#include "io/analog/builder.h"
#include "io/digital/builder.h"
#include "io/touchscreen/builder.h"
#include "io/i2c/builder.h"
#include "io/indicators/builder.h"
#include "io/leds/builder.h"

namespace opendeck::sys
{
    /**
     * @brief Test system backend that accepts initialization and ignores reboot requests.
     */
    class HwaTest : public Hwa
    {
        public:
        HwaTest() = default;

        /**
         * @brief Initializes the test backend.
         *
         * @return Always `true`.
         */
        bool init() override
        {
            return true;
        }

        IoCollection& io() override
        {
            return _io;
        }

        ProtocolCollection& protocol() override
        {
            return _protocol;
        }

        database::Admin& database() override
        {
            return _database;
        }

        /**
         * @brief Ignores reboot requests in tests.
         *
         * @param type Firmware target requested for reboot.
         */
        void reboot(fw_selector::FwType type) override
        {
        }

        database::Builder        _builder_database;
        database::Admin&         _database            = _builder_database.instance();
        io::digital::Builder     _builder_digital     = io::digital::Builder(_database);
        io::analog::Builder      _builder_analog      = io::analog::Builder(_database);
        io::leds::Builder        _builder_leds        = io::leds::Builder(_database);
        io::touchscreen::Builder _builder_touchscreen = io::touchscreen::Builder(_database);
        io::i2c::Builder         _builder_i2c         = io::i2c::Builder(_database);
        io::indicators::Builder  _builder_indicators  = io::indicators::Builder(_database);
        protocol::midi::Builder  _builder_midi        = protocol::midi::Builder(_database);
        IoCollection             _io                  = {
            &_builder_digital.instance(),
            &_builder_analog.instance(),
            &_builder_leds.instance(),
            &_builder_i2c.instance(),
            &_builder_touchscreen.instance(),
            &_builder_indicators.instance(),
        };
        ProtocolCollection _protocol = {
            &_builder_midi.instance(),
        };
    };
}    // namespace opendeck::sys
