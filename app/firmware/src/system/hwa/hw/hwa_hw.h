/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/system/shared/deps.h"
#include "firmware/src/database/builder/builder.h"
#include "firmware/src/mcu/builder/builder.h"
#include "firmware/src/protocol/midi/builder/builder.h"
#include "firmware/src/protocol/osc/builder/builder.h"
#include "firmware/src/protocol/webconfig/builder/builder.h"
#include "firmware/src/protocol/mdns/builder/builder.h"
#include "firmware/src/io/analog/builder/builder.h"
#include "firmware/src/io/digital/builder/builder.h"
#include "firmware/src/io/touchscreen/builder/builder.h"
#include "firmware/src/io/i2c/builder/builder.h"
#include "firmware/src/io/indicators/builder/builder.h"
#include "firmware/src/io/outputs/builder/builder.h"

namespace opendeck::sys
{
    /**
     * @brief Hardware-backed system backend that controls reboot mode selection.
     */
    class HwaHw : public Hwa
    {
        public:
        HwaHw() = default;

        /**
         * @brief Initializes the hardware backend.
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
         * @brief Selects the next boot target and performs a cold reboot.
         *
         * @param type Firmware target to reboot into.
         */
        void reboot(fw_selector::FwType type) override
        {
            _mcu.reboot(type);
        }

        /**
         * @brief Returns the MCU hardware serial-number bytes.
         *
         * @return View into the MCU-owned serial-number bytes, or an empty span when unavailable.
         */
        std::span<uint8_t> serial_number() override
        {
            return _mcu.serial_number();
        }

        private:
        mcu::Builder                 _builder_mcu;
        mcu::Hwa&                    _mcu                 = _builder_mcu.instance();
        database::Admin&             _database            = database::Builder::instance();
        io::digital::Builder         _builder_digital     = io::digital::Builder(_database);
        io::analog::Builder          _builder_analog      = io::analog::Builder(_database);
        io::outputs::Builder         _builder_outputs     = io::outputs::Builder(_database);
        io::touchscreen::Builder     _builder_touchscreen = io::touchscreen::Builder(_database);
        io::i2c::Builder             _builder_i2c         = io::i2c::Builder(_database);
        io::indicators::Builder      _builder_indicators  = io::indicators::Builder(_database);
        protocol::midi::Builder      _builder_midi        = protocol::midi::Builder(_database);
        protocol::osc::Builder       _builder_osc         = protocol::osc::Builder(_database);
        protocol::webconfig::Builder _builder_webconfig   = protocol::webconfig::Builder(_mcu);
        protocol::mdns::Builder      _builder_mdns        = protocol::mdns::Builder(_database, _mcu);
        IoCollection                 _io                  = {
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
            &_builder_webconfig.instance(),
            &_builder_mdns.instance(),
        };
    };
}    // namespace opendeck::sys
