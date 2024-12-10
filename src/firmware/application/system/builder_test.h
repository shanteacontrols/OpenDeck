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
#include "hwa_test.h"
#include "application/io/analog/builder_test.h"
#include "application/io/buttons/builder_test.h"
#include "application/io/encoders/builder_test.h"
#include "application/io/touchscreen/builder_test.h"
#include "application/io/i2c/builder_test.h"
#include "application/io/leds/builder_test.h"
#include "application/database/builder_test.h"
#include "application/protocol/midi/builder_test.h"

namespace sys
{
    class BuilderTest
    {
        public:
        BuilderTest() = default;

        class ComponentsTest : public Components
        {
            public:
            ComponentsTest()
            {
                _io.at(static_cast<size_t>(::io::ioComponent_t::BUTTONS))       = &_builderButtons._instance;
                _io.at(static_cast<size_t>(::io::ioComponent_t::ENCODERS))      = &_builderEncoders._instance;
                _io.at(static_cast<size_t>(::io::ioComponent_t::ANALOG))        = &_builderAnalog._instance;
                _io.at(static_cast<size_t>(::io::ioComponent_t::LEDS))          = &_builderLeds._instance;
                _io.at(static_cast<size_t>(::io::ioComponent_t::I2C))           = &_builderI2c._instance;
                _io.at(static_cast<size_t>(::io::ioComponent_t::TOUCHSCREEN))   = &_builderTouchscreen._instance;
                _protocol.at(static_cast<size_t>(::protocol::protocol_t::MIDI)) = &_builderMidi._instance;
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

            database::BuilderTest                                                          _builderDatabase;
            database::Admin&                                                               _database           = _builderDatabase._instance;
            io::analog::BuilderTest                                                        _builderAnalog      = io::analog::BuilderTest(_database);
            io::encoders::BuilderTest                                                      _builderEncoders    = io::encoders::BuilderTest(_database);
            io::leds::BuilderTest                                                          _builderLeds        = io::leds::BuilderTest(_database);
            io::buttons::BuilderTest                                                       _builderButtons     = io::buttons::BuilderTest(_database);
            io::touchscreen::BuilderTest                                                   _builderTouchscreen = io::touchscreen::BuilderTest(_database);
            io::i2c::BuilderTest                                                           _builderI2c         = io::i2c::BuilderTest(_database);
            protocol::midi::BuilderTest                                                    _builderMidi        = protocol::midi::BuilderTest(_database);
            std::array<io::Base*, static_cast<size_t>(io::ioComponent_t::AMOUNT)>          _io                 = {};
            std::array<protocol::Base*, static_cast<size_t>(protocol::protocol_t::AMOUNT)> _protocol           = {};
        };

        HwaTest        _hwa;
        ComponentsTest _components;
        System         _instance = System(_hwa, _components);
    };
}    // namespace sys
