/*

Copyright 2015-2022 Igor Petrovic

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

#include "system/System.h"
#include "io/buttons/Buttons.h"
#include "io/buttons/Filter.h"
#include "io/encoders/Encoders.h"
#include "io/encoders/Filter.h"
#include "io/analog/Analog.h"
#include "io/analog/Filter.h"
#include "io/leds/LEDs.h"
#include "io/i2c/I2C.h"
#include "io/i2c/peripherals/Builder.h"
#include "io/touchscreen/Touchscreen.h"
#include "io/touchscreen/model/Builder.h"
#include "protocol/dmx/DMX.h"
#include "protocol/midi/MIDI.h"
#include "database/Layout.h"

namespace System
{
    class Builder
    {
        public:
        class HWA
        {
            public:
            class IO
            {
                // these are just renames of existing HWAs in order to have them in the same namespace
                public:
                using Analog         = ::IO::Analog::HWA;
                using Buttons        = ::IO::Buttons::HWA;
                using CDCPassthrough = ::IO::Touchscreen::CDCPassthrough;
                using Display        = ::IO::I2C::Peripheral::HWA;
                using Encoders       = ::IO::Encoders::HWA;
                using LEDs           = ::IO::LEDs::HWA;
                using Touchscreen    = ::IO::Touchscreen::HWA;

                virtual Analog&         analog()         = 0;
                virtual Buttons&        buttons()        = 0;
                virtual CDCPassthrough& cdcPassthrough() = 0;
                virtual Display&        display()        = 0;
                virtual Encoders&       encoders()       = 0;
                virtual LEDs&           leds()           = 0;
                virtual Touchscreen&    touchscreen()    = 0;
            };

            class Protocol
            {
                public:
                class MIDI
                {
                    public:
                    using USB = ::Protocol::MIDI::HWAUSB;
                    using DIN = ::Protocol::MIDI::HWADIN;
                    using BLE = ::Protocol::MIDI::HWABLE;

                    virtual USB& usb() = 0;
                    virtual DIN& din() = 0;
                    virtual BLE& ble() = 0;
                };

                using DMX = ::Protocol::DMX::HWA;

                virtual MIDI& midi() = 0;
                virtual DMX&  dmx()  = 0;
            };

            using Database = ::Database::Admin::StorageAccess;
            using System   = ::System::Instance::HWA;

            virtual IO&       io()       = 0;
            virtual Protocol& protocol() = 0;
            virtual System&   system()   = 0;
            virtual Database& database() = 0;
        };

        Builder(HWA& hwa)
            : _hwa(hwa)
            , _components(*this)
        {
            _io.at(static_cast<size_t>(::IO::ioComponent_t::BUTTONS))     = &_buttons;
            _io.at(static_cast<size_t>(::IO::ioComponent_t::ENCODERS))    = &_encoders;
            _io.at(static_cast<size_t>(::IO::ioComponent_t::ANALOG))      = &_analog;
            _io.at(static_cast<size_t>(::IO::ioComponent_t::LEDS))        = &_leds;
            _io.at(static_cast<size_t>(::IO::ioComponent_t::I2C))         = &_i2c;
            _io.at(static_cast<size_t>(::IO::ioComponent_t::TOUCHSCREEN)) = &_touchscreen;

            _protocol.at(static_cast<size_t>(::Protocol::protocol_t::MIDI)) = &_midi;
            _protocol.at(static_cast<size_t>(::Protocol::protocol_t::DMX))  = &_dmx;
        }

        System::Instance::Components& components()
        {
            return _components;
        }

        System::Instance::HWA& hwa()
        {
            return _hwa.system();
        }

        private:
        HWA&                                                                           _hwa;
        Database::AppLayout                                                            _dbLayout;
        Database::Admin                                                                _database            = Database::Admin(_hwa.database(), _dbLayout, DATABASE_INIT_DATA);
        IO::Buttons::Database                                                          _buttonsDatabase     = IO::Buttons::Database(_database);
        IO::Analog::Database                                                           _analogDatabase      = IO::Analog::Database(_database);
        IO::Encoders::Database                                                         _encodersDatabase    = IO::Encoders::Database(_database);
        IO::LEDs::Database                                                             _ledsDatabase        = IO::LEDs::Database(_database);
        IO::Touchscreen::Database                                                      _touchscreenDatabase = IO::Touchscreen::Database(_database);
        IO::I2CPeripheralBuilder::DisplayDatabase                                      _displayDatabase     = IO::I2CPeripheralBuilder::DisplayDatabase(_database);
        Protocol::MIDI::Database                                                       _midiDatabase        = Protocol::MIDI::Database(_database);
        Protocol::DMX::Database                                                        _dmxDatabase         = Protocol::DMX::Database(_database);
        IO::EncodersFilter                                                             _encodersFilter;
        IO::ButtonsFilter                                                              _buttonsFilter;
        IO::AnalogFilter                                                               _analogFilter;
        IO::Analog                                                                     _analog            = IO::Analog(_hwa.io().analog(), _analogFilter, _analogDatabase);
        IO::Buttons                                                                    _buttons           = IO::Buttons(_hwa.io().buttons(), _buttonsFilter, _buttonsDatabase);
        IO::LEDs                                                                       _leds              = IO::LEDs(_hwa.io().leds(), _ledsDatabase);
        IO::Encoders                                                                   _encoders          = IO::Encoders(_hwa.io().encoders(), _encodersFilter, _encodersDatabase, 1);
        IO::Touchscreen                                                                _touchscreen       = IO::Touchscreen(_hwa.io().touchscreen(), _touchscreenDatabase, _hwa.io().cdcPassthrough());
        IO::TouchscreenModelBuilder                                                    _touchscreenModels = IO::TouchscreenModelBuilder(_hwa.io().touchscreen());
        IO::I2C                                                                        _i2c;
        IO::I2CPeripheralBuilder                                                       _i2cPeripherals = IO::I2CPeripheralBuilder(_hwa.io().display(), _displayDatabase);
        Protocol::MIDI                                                                 _midi           = Protocol::MIDI(_hwa.protocol().midi().usb(), _hwa.protocol().midi().din(), _hwa.protocol().midi().ble(), _midiDatabase);
        Protocol::DMX                                                                  _dmx            = Protocol::DMX(_hwa.protocol().dmx(), _dmxDatabase);
        std::array<IO::Base*, static_cast<size_t>(IO::ioComponent_t::AMOUNT)>          _io             = {};
        std::array<Protocol::Base*, static_cast<size_t>(Protocol::protocol_t::AMOUNT)> _protocol       = {};

        class Components : public System::Instance::Components
        {
            public:
            Components(Builder& builder)
                : _builder(builder)
            {}

            std::array<IO::Base*, static_cast<size_t>(IO::ioComponent_t::AMOUNT)>& io() override
            {
                return _builder._io;
            }

            std::array<Protocol::Base*, static_cast<size_t>(Protocol::protocol_t::AMOUNT)>& protocol() override
            {
                return _builder._protocol;
            }

            Database::Admin& database() override
            {
                return _builder._database;
            }

            private:
            Builder& _builder;
        };

        Components _components;
    };
}    // namespace System
