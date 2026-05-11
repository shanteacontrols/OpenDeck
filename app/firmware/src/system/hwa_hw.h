/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "retained/retained.h"
#include "database/builder.h"
#include "protocol/midi/builder.h"
#include "protocol/osc/builder.h"
#include "protocol/webconfig/builder.h"
#include "protocol/mdns/builder.h"
#include "io/analog/builder.h"
#include "io/digital/builder.h"
#include "io/touchscreen/builder.h"
#include "io/i2c/builder.h"
#include "io/indicators/builder.h"
#include "io/leds/builder.h"

#include <zephyr/sys/reboot.h>

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
            retained::data.boot_mode.set(static_cast<uint32_t>(type));
            sys_reboot(SYS_REBOOT_COLD);
        }

        private:
        database::Admin&             _database            = database::Builder::instance();
        io::digital::Builder         _builder_digital     = io::digital::Builder(_database);
        io::analog::Builder          _builder_analog      = io::analog::Builder(_database);
        io::leds::Builder            _builder_leds        = io::leds::Builder(_database);
        io::touchscreen::Builder     _builder_touchscreen = io::touchscreen::Builder(_database);
        io::i2c::Builder             _builder_i2c         = io::i2c::Builder(_database);
        io::indicators::Builder      _builder_indicators  = io::indicators::Builder(_database);
        protocol::midi::Builder      _builder_midi        = protocol::midi::Builder(_database);
        protocol::osc::Builder       _builder_osc         = protocol::osc::Builder(_database);
        protocol::webconfig::Builder _builder_webconfig;
        protocol::mdns::Builder      _builder_mdns = protocol::mdns::Builder(_database);
        IoCollection                 _io           = {
            &_builder_digital.instance(),
            &_builder_analog.instance(),
            &_builder_leds.instance(),
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
