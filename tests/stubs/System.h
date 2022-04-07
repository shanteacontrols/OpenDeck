#pragma once

#include "HWALEDs.h"
#include "HWAAnalog.h"
#include "HWAButtons.h"
#include "HWAEncoders.h"
#include "HWATouchscreen.h"
#include "HWASystem.h"
#include "HWAI2C.h"
#include "HWAMIDI.h"
#include "HWADMX.h"
#include "Database.h"
#include "system/Builder.h"
#include "system/System.h"

class TestSystem
{
    public:
    TestSystem()
        : _hwa(*this)
    {}

    public:
    DBstorageMock                _hwaDatabase;
    HWALEDs                      _hwaLEDs;
    HWAAnalog                    _hwaAnalog;
    HWAButtons                   _hwaButtons;
    HWAEncoders                  _hwaEncoders;
    HWATouchscreen               _hwaTouchscreen;
    HWATouchscreenCDCPassthrough _hwaCDCPassthrough;
    HWAI2C                       _hwaI2C;
    HWASystem                    _hwaSystem;
    HWAMIDIUSB                   _hwaMIDIUSB;
    HWAMIDIDIN                   _hwaMIDIDIN;
    HWAMIDIBLE                   _hwaMIDIBLE;
    HWADMX                       _hwaDMX;

    private:
    class HWABuilder : public ::System::Builder::HWA
    {
        public:
        HWABuilder(TestSystem& testSystem)
            : _testSystem(testSystem)
            , _io(testSystem)
            , _protocol(testSystem)
        {}

        ::System::Builder::HWA::IO& io() override
        {
            return _io;
        }

        ::System::Builder::HWA::Protocol& protocol() override
        {
            return _protocol;
        }

        ::System::Builder::HWA::System& system() override
        {
            return _testSystem._hwaSystem;
        }

        ::System::Builder::HWA::Database& database() override
        {
            return _testSystem._hwaDatabase;
        }

        class HWAIO : public ::System::Builder::HWA::IO
        {
            public:
            HWAIO(TestSystem& testSystem)
                : _testSystem(testSystem)
            {}

            ::System::Builder::HWA::IO::LEDs& leds() override
            {
                return _testSystem._hwaLEDs;
            }

            ::System::Builder::HWA::IO::Analog& analog() override
            {
                return _testSystem._hwaAnalog;
            }

            ::System::Builder::HWA::IO::Buttons& buttons() override
            {
                return _testSystem._hwaButtons;
            }

            ::System::Builder::HWA::IO::Encoders& encoders() override
            {
                return _testSystem._hwaEncoders;
            }

            ::System::Builder::HWA::IO::Touchscreen& touchscreen() override
            {
                return _testSystem._hwaTouchscreen;
            }

            ::System::Builder::HWA::IO::CDCPassthrough& cdcPassthrough() override
            {
                return _testSystem._hwaCDCPassthrough;
            }

            ::System::Builder::HWA::IO::I2C& i2c() override
            {
                return _testSystem._hwaI2C;
            }

            private:
            TestSystem& _testSystem;
        };

        class HWAProtocol : public ::System::Builder::HWA::Protocol
        {
            public:
            HWAProtocol(TestSystem& testSystem)
                : _testSystem(testSystem)
                , _hwaMIDI(testSystem)
            {}

            ::System::Builder::HWA::Protocol::MIDI& midi()
            {
                return _hwaMIDI;
            }

            ::System::Builder::HWA::Protocol::DMX& dmx()
            {
                return _testSystem._hwaDMX;
            }

            class HWAProtocolMIDI : public ::System::Builder::HWA::Protocol::MIDI
            {
                public:
                HWAProtocolMIDI(TestSystem& testSystem)
                    : _testSystem(testSystem)
                {}

                ::System::Builder::HWA::Protocol::MIDI::USB& usb() override
                {
                    return _testSystem._hwaMIDIUSB;
                }

                ::System::Builder::HWA::Protocol::MIDI::DIN& din() override
                {
                    return _testSystem._hwaMIDIDIN;
                }

                ::System::Builder::HWA::Protocol::MIDI::BLE& ble() override
                {
                    return _testSystem._hwaMIDIBLE;
                }

                private:
                TestSystem& _testSystem;
            };

            private:
            TestSystem&     _testSystem;
            HWAProtocolMIDI _hwaMIDI;
        };

        TestSystem& _testSystem;
        HWAIO       _io;
        HWAProtocol _protocol;
    };

    HWABuilder      _hwa;
    System::Builder _builder = System::Builder(_hwa);

    public:
    ::System::Instance _instance = ::System::Instance(_builder.hwa(), _builder.components());
};