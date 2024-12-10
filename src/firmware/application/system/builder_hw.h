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
#include "application/io/analog/builder_hw.h"
#include "application/io/buttons/builder_hw.h"
#include "application/io/encoders/builder_hw.h"
#include "application/io/touchscreen/builder_hw.h"
#include "application/io/i2c/builder_hw.h"
#include "application/io/leds/builder_hw.h"
#include "application/database/builder_hw.h"
#include "application/protocol/midi/builder_hw.h"

namespace sys
{
    class BuilderHw
    {
        public:
        BuilderHw() = default;

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
            io::analog::BuilderHw                                                          _builderAnalog;
            io::encoders::BuilderHw                                                        _builderEncoders;
            io::leds::BuilderHw                                                            _builderLeds;
            io::buttons::BuilderHw                                                         _builderButtons;
            io::touchscreen::BuilderHw                                                     _builderTouchscreen;
            io::i2c::BuilderHw                                                             _builderI2c;
            protocol::midi::BuilderHw                                                      _builderMidi;
            std::array<io::Base*, static_cast<size_t>(io::ioComponent_t::AMOUNT)>          _io       = {};
            std::array<protocol::Base*, static_cast<size_t>(protocol::protocol_t::AMOUNT)> _protocol = {};
            database::Admin&                                                               _database = database::BuilderHw::instance();
        };

        HwaHw        _hwa;
        ComponentsHw _components;
        System       _instance = System(_hwa, _components);
    };
}    // namespace sys
