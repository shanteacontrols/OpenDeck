#pragma once

#include "HWALEDs.h"
#include "HWAAnalog.h"
#include "HWAButtons.h"
#include "HWAEncoders.h"
#include "HWATouchscreen.h"
#include "HWASystem.h"
#include "HWADisplay.h"
#include "HWAMIDI.h"
#include "Database.h"
#include "application/system/Builder.h"
#include "application/system/System.h"

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
    HWADisplay                   _hwaDisplay;
    HWASystem                    _hwaSystem;
    HWAMIDIUSB                   _hwaMIDIUSB;
    HWAMIDIDIN                   _hwaMIDIDIN;
    HWAMIDIBLE                   _hwaMIDIBLE;

    private:
    class HWABuilder : public ::sys::Builder::HWA
    {
        public:
        HWABuilder(TestSystem& testSystem)
            : _testSystem(testSystem)
            , _io(testSystem)
            , _protocol(testSystem)
        {}

        ::sys::Builder::HWA::IO& io() override
        {
            return _io;
        }

        ::sys::Builder::HWA::Protocol& protocol() override
        {
            return _protocol;
        }

        ::sys::Builder::HWA::System& system() override
        {
            return _testSystem._hwaSystem;
        }

        ::sys::Builder::HWA::Database& database() override
        {
            return _testSystem._hwaDatabase;
        }

        class HWAIO : public ::sys::Builder::HWA::IO
        {
            public:
            HWAIO(TestSystem& testSystem)
                : _testSystem(testSystem)
            {}

            ::sys::Builder::HWA::IO::LEDs& leds() override
            {
                return _testSystem._hwaLEDs;
            }

            ::sys::Builder::HWA::IO::Analog& analog() override
            {
                return _testSystem._hwaAnalog;
            }

            ::sys::Builder::HWA::IO::Buttons& buttons() override
            {
                return _testSystem._hwaButtons;
            }

            ::sys::Builder::HWA::IO::Encoders& encoders() override
            {
                return _testSystem._hwaEncoders;
            }

            ::sys::Builder::HWA::IO::Touchscreen& touchscreen() override
            {
                return _testSystem._hwaTouchscreen;
            }

            ::sys::Builder::HWA::IO::CDCPassthrough& cdcPassthrough() override
            {
                return _testSystem._hwaCDCPassthrough;
            }

            ::sys::Builder::HWA::IO::Display& display() override
            {
                return _testSystem._hwaDisplay;
            }

            private:
            TestSystem& _testSystem;
        };

        class HWAProtocol : public ::sys::Builder::HWA::Protocol
        {
            public:
            HWAProtocol(TestSystem& testSystem)
                : _hwaMIDI(testSystem)
            {}

            ::sys::Builder::HWA::Protocol::MIDI& midi()
            {
                return _hwaMIDI;
            }

            class HWAProtocolMIDI : public ::sys::Builder::HWA::Protocol::MIDI
            {
                public:
                HWAProtocolMIDI(TestSystem& testSystem)
                    : _testSystem(testSystem)
                {}

                ::sys::Builder::HWA::Protocol::MIDI::USB& usb() override
                {
                    return _testSystem._hwaMIDIUSB;
                }

                ::sys::Builder::HWA::Protocol::MIDI::DIN& din() override
                {
                    return _testSystem._hwaMIDIDIN;
                }

                ::sys::Builder::HWA::Protocol::MIDI::BLE& ble() override
                {
                    return _testSystem._hwaMIDIBLE;
                }

                private:
                TestSystem& _testSystem;
            };

            private:
            HWAProtocolMIDI _hwaMIDI;
        };

        TestSystem& _testSystem;
        HWAIO       _io;
        HWAProtocol _protocol;
    };

    HWABuilder   _hwa;
    sys::Builder _builder = sys::Builder(_hwa);

    public:
    ::sys::Instance _instance = ::sys::Instance(_builder.hwa(), _builder.components());
};