/*

Copyright Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#pragma once

#include "system.h"
#include "hwa_hw.h"
#include "application/database/builder.h"
#include "application/protocol/midi/builder.h"
#include "application/io/analog/builder.h"
#include "application/io/buttons/builder.h"
#include "application/io/encoders/builder.h"
#include "application/io/touchscreen/builder.h"
#include "application/io/i2c/builder.h"
#include "application/io/leds/builder.h"

namespace sys
{
    class Builder
    {
        public:
        Builder() = default;

        System& instance()
        {
            return _instance;
        }

        private:
        class ComponentsHw : public Components
        {
            public:
            ComponentsHw()
            {
                _io.at(static_cast<size_t>(::io::ioComponent_t::BUTTONS))       = &_builderButtons.instance();
                _io.at(static_cast<size_t>(::io::ioComponent_t::ENCODERS))      = &_builderEncoders.instance();
                _io.at(static_cast<size_t>(::io::ioComponent_t::ANALOG))        = &_builderAnalog.instance();
                _io.at(static_cast<size_t>(::io::ioComponent_t::LEDS))          = &_builderLeds.instance();
                _io.at(static_cast<size_t>(::io::ioComponent_t::I2C))           = &_builderI2c.instance();
                _io.at(static_cast<size_t>(::io::ioComponent_t::TOUCHSCREEN))   = &_builderTouchscreen.instance();
                _protocol.at(static_cast<size_t>(::protocol::protocol_t::MIDI)) = &_builderMidi.instance();
            }

            ioComponents_t& io() override
            {
                return _io;
            }

            protocolComponents_t& protocol() override
            {
                return _protocol;
            }

            database::Admin& database() override
            {
                return _database;
            }

            private:
            database::Builder                                                              _builderDatabase;
            database::Admin&                                                               _database           = _builderDatabase.instance();
            io::analog::Builder                                                            _builderAnalog      = io::analog::Builder(_database);
            io::encoders::Builder                                                          _builderEncoders    = io::encoders::Builder(_database);
            io::leds::Builder                                                              _builderLeds        = io::leds::Builder(_database);
            io::buttons::Builder                                                           _builderButtons     = io::buttons::Builder(_database);
            io::touchscreen::Builder                                                       _builderTouchscreen = io::touchscreen::Builder(_database);
            io::i2c::Builder                                                               _builderI2c         = io::i2c::Builder(_database);
            protocol::midi::Builder                                                        _builderMidi        = protocol::midi::Builder(_database);
            std::array<io::Base*, static_cast<size_t>(io::ioComponent_t::AMOUNT)>          _io                 = {};
            std::array<protocol::Base*, static_cast<size_t>(protocol::protocol_t::AMOUNT)> _protocol           = {};
        };

        HwaHw        _hwa;
        ComponentsHw _components;
        System       _instance = System(_hwa, _components);
    };
}    // namespace sys
