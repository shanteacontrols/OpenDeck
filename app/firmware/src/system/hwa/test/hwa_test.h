/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/system/shared/deps.h"
#include "firmware/src/database/builder/builder.h"
#include "common/src/mcu/builder/builder.h"
#include "common/src/mcu/shared/common.h"
#include "firmware/src/protocol/midi/builder/builder.h"
#include "firmware/src/protocol/osc/builder/builder.h"
#include "firmware/src/protocol/websockets/builder/builder.h"
#include "firmware/src/protocol/mdns/builder/builder.h"
#include "firmware/src/io/analog/builder/builder.h"
#include "firmware/src/io/digital/builder/builder.h"
#include "firmware/src/io/touchscreen/builder/builder.h"
#include "firmware/src/io/i2c/builder/builder.h"
#include "firmware/src/io/indicators/builder/builder.h"
#include "firmware/src/io/outputs/builder/builder.h"

#include <array>

namespace opendeck::firmware::sys
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
        void reboot(opendeck::common::mcu::BootTarget type) override
        {
            _mcu.reboot(type);
        }

        /**
         * @brief Returns the test MCU serial-number bytes.
         *
         * @return View into the configured test serial-number bytes.
         */
        std::span<uint8_t> serial_number() override
        {
            return _mcu.serial_number();
        }

        opendeck::common::mcu::Builder                                         _builder_mcu;
        opendeck::common::mcu::Hwa&                                            _mcu   = _builder_mcu.instance();
        std::array<uint8_t, opendeck::common::mcu::SERIAL_NUMBER_BUFFER_SIZE>& serial = _builder_mcu._hwa.serial;
        database::Builder                                                      _builder_database;
        database::Admin&                                                       _database            = _builder_database.instance();
        io::digital::Builder                                                   _builder_digital     = io::digital::Builder(_database);
        io::analog::Builder                                                    _builder_analog      = io::analog::Builder(_database);
        io::outputs::Builder                                                   _builder_outputs     = io::outputs::Builder(_database);
        io::touchscreen::Builder                                               _builder_touchscreen = io::touchscreen::Builder(_database);
        io::i2c::Builder                                                       _builder_i2c         = io::i2c::Builder(_database);
        io::indicators::Builder                                                _builder_indicators  = io::indicators::Builder(_database);
        protocol::midi::Builder                                                _builder_midi        = protocol::midi::Builder(_database);
        protocol::osc::Builder                                                 _builder_osc         = protocol::osc::Builder(_database);
        protocol::websockets::Builder                                          _builder_websockets;
        protocol::mdns::Builder                                                _builder_mdns = protocol::mdns::Builder(_database);
        IoCollection                                                           _io           = {
            &_builder_digital.instance(),
            &_builder_analog.instance(),
            &_builder_outputs.instance(),
            &_builder_i2c.instance(),
            &_builder_touchscreen.instance(),
            &_builder_indicators.instance(),
        };
        ProtocolCollection _protocol = {
            &_builder_midi.instance(),
            &_builder_osc.instance(),
            &_builder_websockets.instance(),
            &_builder_mdns.instance(),
        };
    };
}    // namespace opendeck::firmware::sys
