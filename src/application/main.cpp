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

#include "core/src/Timing.h"
#include "core/src/MCU.h"
#include "board/Board.h"
#include "io/common/Common.h"
#include "system/System.h"
#include "system/Builder.h"
#ifdef USE_LOGGER
#include "logger/Logger.h"
#endif

class CDCLocker
{
    public:
    CDCLocker() = default;

    static bool locked()
    {
        return _locked;
    }

    static void lock()
    {
        _locked = true;
    }

    static void unlock()
    {
        _locked = false;
    }

    private:
    static bool _locked;
};

bool CDCLocker::_locked = false;

class HWADatabase : public System::Builder::HWA::Database
{
    public:
    HWADatabase() = default;

    bool init() override
    {
        return Board::NVM::init();
    }

    uint32_t size() override
    {
        return Board::NVM::size();
    }

    bool clear() override
    {
#ifdef HW_SUPPORT_LED_INDICATORS
        // It's possible that LED indicators are still on since
        // this command is most likely given via USB.
        // Wait until all indicators are turned off
        core::timing::waitMs(Board::IO::indicators::LED_TRAFFIC_INDICATOR_TIMEOUT);
#endif

        return Board::NVM::clear(0, Board::NVM::size());
    }

    bool read(uint32_t address, uint32_t& value, Database::Admin::sectionParameterType_t type) override
    {
        return Board::NVM::read(address, value, boardParamType(type));
    }

    bool write(uint32_t address, uint32_t value, Database::Admin::sectionParameterType_t type) override
    {
        return Board::NVM::write(address, value, boardParamType(type));
    }

    private:
    Board::NVM::parameterType_t boardParamType(Database::Admin::sectionParameterType_t type)
    {
        switch (type)
        {
        case Database::Admin::sectionParameterType_t::WORD:
            return Board::NVM::parameterType_t::WORD;

        case Database::Admin::sectionParameterType_t::DWORD:
            return Board::NVM::parameterType_t::DWORD;

        default:
            return Board::NVM::parameterType_t::BYTE;
        }
    }
} _hwaDatabase;

#ifdef LEDS_SUPPORTED
class HWALEDs : public System::Builder::HWA::IO::LEDs
{
    public:
    HWALEDs() = default;

    void setState(size_t index, IO::LEDs::brightness_t brightness) override
    {
        Board::IO::digitalOut::writeLEDstate(index, static_cast<Board::IO::digitalOut::ledBrightness_t>(brightness));
    }

    size_t rgbFromOutput(size_t index) override
    {
        return Board::IO::digitalOut::rgbFromOutput(index);
    }

    size_t rgbComponentFromRGB(size_t index, IO::LEDs::rgbComponent_t component) override
    {
        return Board::IO::digitalOut::rgbComponentFromRGB(index,
                                                          static_cast<Board::IO::digitalOut::rgbComponent_t>(component));
    }
} _hwaLEDs;
#else
class HWALEDsStub : public System::Builder::HWA::IO::LEDs
{
    public:
    HWALEDsStub() = default;

    void setState(size_t index, IO::LEDs::brightness_t brightness) override
    {
    }

    size_t rgbFromOutput(size_t index) override
    {
        return 0;
    }

    size_t rgbComponentFromRGB(size_t index, IO::LEDs::rgbComponent_t rgbComponent) override
    {
        return 0;
    }

    void registerHandler(void (*fptr)(size_t index, bool state))
    {
    }
} _hwaLEDs;
#endif

#ifdef HW_SUPPORT_ADC
class HWAAnalog : public System::Builder::HWA::IO::Analog
{
    public:
    HWAAnalog() = default;

    bool value(size_t index, uint16_t& value) override
    {
        return Board::IO::analog::value(index, value);
    }
} _hwaAnalog;

#else
class HWAAnalogStub : public System::Builder::HWA::IO::Analog
{
    public:
    HWAAnalogStub() = default;

    bool value(size_t index, uint16_t& value) override
    {
        return false;
    }
} _hwaAnalog;
#endif

#if defined(BUTTONS_SUPPORTED) || defined(ENCODERS_SUPPORTED)
// buttons and encoders have the same data source which is digital input
// this helper class pulls the latest data from board and then feeds it into HWAButtons and HWAEncoders
class HWADigitalIn
{
    public:
    HWADigitalIn() = default;

#ifdef BUTTONS_SUPPORTED
    bool buttonState(size_t index, uint8_t& numberOfReadings, uint32_t& states)
    {
        if (!Board::IO::digitalIn::state(index, _dInReadA))
        {
            return false;
        }

        numberOfReadings = _dInReadA.count;
        states           = _dInReadA.readings;

        return true;
    }
#endif

#ifdef ENCODERS_SUPPORTED
    bool encoderState(size_t index, uint8_t& numberOfReadings, uint32_t& states)
    {
        if (!Board::IO::digitalIn::state(Board::IO::digitalIn::encoderComponentFromEncoder(index,
                                                                                           Board::IO::digitalIn::encoderComponent_t::A),
                                         _dInReadA))
        {
            return false;
        }

        if (!Board::IO::digitalIn::state(Board::IO::digitalIn::encoderComponentFromEncoder(index,
                                                                                           Board::IO::digitalIn::encoderComponent_t::B),
                                         _dInReadB))
        {
            return false;
        }

        numberOfReadings = _dInReadA.count > _dInReadB.count ? _dInReadA.count : _dInReadB.count;

        // construct encoder pair readings
        // encoder signal is made of A and B signals
        // take each bit of A signal and B signal and append it to states variable in order
        // latest encoder readings should be in LSB bits

        for (uint8_t i = 0; i < numberOfReadings; i++)
        {
            core::util::BIT_WRITE(states, (i * 2) + 1, (_dInReadA.readings >> i & 0x01));
            core::util::BIT_WRITE(states, i * 2, (_dInReadB.readings >> i & 0x01));
        }

        return true;
    }
#endif

    private:
    Board::IO::digitalIn::readings_t _dInReadA;
#ifdef ENCODERS_SUPPORTED
    Board::IO::digitalIn::readings_t _dInReadB;
#endif
} _hwaDigitalIn;
#endif

#ifdef BUTTONS_SUPPORTED

class HWAButtons : public System::Builder::HWA::IO::Buttons
{
    public:
    HWAButtons() = default;

    bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
    {
        return _hwaDigitalIn.buttonState(index, numberOfReadings, states);
    }

    size_t buttonToEncoderIndex(size_t index) override
    {
        return Board::IO::digitalIn::encoderFromInput(index);
    }
} _hwaButtons;
#else
class HWAButtonsStub : public System::Builder::HWA::IO::Buttons
{
    public:
    HWAButtonsStub() = default;

    bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
    {
        return false;
    }

    size_t buttonToEncoderIndex(size_t index) override
    {
        return 0;
    }
} _hwaButtons;
#endif

#ifdef ENCODERS_SUPPORTED

class HWAEncoders : public System::Builder::HWA::IO::Encoders
{
    public:
    HWAEncoders() = default;

    bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
    {
        return _hwaDigitalIn.encoderState(index, numberOfReadings, states);
    }
} _hwaEncoders;
#else
class HWAEncodersStub : public System::Builder::HWA::IO::Encoders
{
    public:
    HWAEncodersStub() = default;

    bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
    {
        return false;
    }
} _hwaEncoders;
#endif

class HWAMIDIUSB : public System::Builder::HWA::Protocol::MIDI::USB
{
    public:
    HWAMIDIUSB() = default;

    bool supported() override
    {
        return true;
    }

    bool init() override
    {
        // usb has both MIDI and CDC interface - it might be already initialized
        return Board::USB::init() != Board::initStatus_t::ERROR;
    }

    bool deInit() override
    {
        return true;    // never deinit usb interface, just pretend here
    }

    bool read(MIDI::usbMIDIPacket_t& packet) override
    {
        if (Board::USB::readMIDI(packet))
        {
            Board::IO::indicators::indicateTraffic(Board::IO::indicators::source_t::USB,
                                                   Board::IO::indicators::direction_t::INCOMING);

            return true;
        }

        return false;
    }

    bool write(MIDI::usbMIDIPacket_t& packet) override
    {
        if (Board::USB::writeMIDI(packet))
        {
            Board::IO::indicators::indicateTraffic(Board::IO::indicators::source_t::USB,
                                                   Board::IO::indicators::direction_t::OUTGOING);

            return true;
        }

        return false;
    }
} _hwaMIDIUSB;

#ifdef HW_SUPPORT_DIN_MIDI
class HWAMIDIDIN : public System::Builder::HWA::Protocol::MIDI::DIN
{
    public:
    HWAMIDIDIN() = default;

    bool supported() override
    {
        return true;
    }

    bool init() override
    {
        static constexpr uint32_t BAUDRATE = 31250;
        return Board::UART::init(HW_UART_CHANNEL_DIN, BAUDRATE) == Board::initStatus_t::OK;
    }

    bool deInit() override
    {
        Board::UART::deInit(HW_UART_CHANNEL_DIN);
        return true;
    }

    bool setLoopback(bool state) override
    {
        Board::UART::setLoopbackState(HW_UART_CHANNEL_DIN, state);
        return true;
    }

    bool read(uint8_t& value) override
    {
        if (Board::UART::read(HW_UART_CHANNEL_DIN, value))
        {
            Board::IO::indicators::indicateTraffic(Board::IO::indicators::source_t::UART,
                                                   Board::IO::indicators::direction_t::INCOMING);

            return true;
        }

        return false;
    }

    bool write(uint8_t& value) override
    {
        if (Board::UART::write(HW_UART_CHANNEL_DIN, value))
        {
            Board::IO::indicators::indicateTraffic(Board::IO::indicators::source_t::UART,
                                                   Board::IO::indicators::direction_t::OUTGOING);

            return true;
        }

        return false;
    }

    bool allocated(IO::Common::Allocatable::interface_t interface) override
    {
        if (interface == IO::Common::Allocatable::interface_t::UART)
        {
            return Board::UART::isInitialized(HW_UART_CHANNEL_DIN);
        }

        return false;
    }
} _hwaMIDIDIN;
#else
class HWAMIDIDINStub : public System::Builder::HWA::Protocol::MIDI::DIN
{
    public:
    HWAMIDIDINStub() = default;

    bool supported() override
    {
        return false;
    }

    bool init() override
    {
        return false;
    }

    bool deInit() override
    {
        return false;
    }

    bool setLoopback(bool state) override
    {
        return false;
    }

    bool read(uint8_t& value) override
    {
        return false;
    }

    bool write(uint8_t& value) override
    {
        return false;
    }

    bool allocated(IO::Common::Allocatable::interface_t interface) override
    {
        return false;
    }
} _hwaMIDIDIN;
#endif

#ifdef HW_SUPPORT_BLE
class HWAMIDIBLE : public System::Builder::HWA::Protocol::MIDI::BLE
{
    public:
    HWAMIDIBLE() = default;

    bool supported() override
    {
        return true;
    }

    bool init() override
    {
        return Board::BLE::init();
    }

    bool deInit() override
    {
        return Board::BLE::deInit();
    }

    bool write(MIDIlib::BLEMIDI::bleMIDIPacket_t& packet) override
    {
        Board::IO::indicators::indicateTraffic(Board::IO::indicators::source_t::BLE,
                                               Board::IO::indicators::direction_t::OUTGOING);

        return Board::BLE::MIDI::write(&packet.data[0], packet.size);
    }

    bool read(MIDIlib::BLEMIDI::bleMIDIPacket_t& packet) override
    {
        if (Board::BLE::MIDI::read(&packet.data[0], packet.size, packet.data.size()))
        {
            Board::IO::indicators::indicateTraffic(Board::IO::indicators::source_t::BLE,
                                                   Board::IO::indicators::direction_t::INCOMING);

            return true;
        }

        return false;
    }

    uint32_t time() override
    {
        return core::timing::ms();
    }
} _hwaMIDIBLE;
#else
class HWAMIDIBLEStub : public System::Builder::HWA::Protocol::MIDI::BLE
{
    public:
    HWAMIDIBLEStub() = default;

    bool supported() override
    {
        return false;
    }

    bool init() override
    {
        return false;
    }

    bool deInit() override
    {
        return false;
    }

    bool write(MIDIlib::BLEMIDI::bleMIDIPacket_t& packet) override
    {
        return false;
    }

    bool read(MIDIlib::BLEMIDI::bleMIDIPacket_t& data) override
    {
        return false;
    }

    uint32_t time() override
    {
        return 0;
    }
} _hwaMIDIBLE;
#endif

#ifdef HW_SUPPORT_TOUCHSCREEN
class HWATouchscreen : public System::Builder::HWA::IO::Touchscreen
{
    public:
    HWATouchscreen() = default;

    bool init() override
    {
        static constexpr uint32_t BAUDRATE = 38400;
        return Board::UART::init(HW_UART_CHANNEL_TOUCHSCREEN, BAUDRATE) == Board::initStatus_t::OK;
    }

    bool deInit() override
    {
        return Board::UART::deInit(HW_UART_CHANNEL_TOUCHSCREEN);
    }

    bool write(uint8_t value) override
    {
        return Board::UART::write(HW_UART_CHANNEL_TOUCHSCREEN, value);
    }

    bool read(uint8_t& value) override
    {
        return Board::UART::read(HW_UART_CHANNEL_TOUCHSCREEN, value);
    }

    bool allocated(IO::Common::Allocatable::interface_t interface) override
    {
#ifdef HW_UART_CHANNEL_TOUCHSCREEN
        return Board::UART::isInitialized(HW_UART_CHANNEL_TOUCHSCREEN);
#else
        return false;
#endif
    }
} _hwaTouchscreen;
#else
class HWATouchscreenStub : public System::Builder::HWA::IO::Touchscreen
{
    public:
    HWATouchscreenStub() = default;

    bool init() override
    {
        return false;
    }

    bool deInit() override
    {
        return false;
    }

    bool write(uint8_t value) override
    {
        return false;
    }

    bool read(uint8_t& value) override
    {
        return false;
    }

    bool allocated(IO::Common::Allocatable::interface_t interface) override
    {
        return false;
    }
} _hwaTouchscreen;
#endif

#if defined(HW_SUPPORT_TOUCHSCREEN) && !defined(HW_USB_OVER_SERIAL)
// USB link MCUs don't implement USB-CDC<->UART passthrough
class HWACDCPassthrough : public System::Builder::HWA::IO::CDCPassthrough
{
    public:
    HWACDCPassthrough() = default;

    bool supported() override
    {
        return true;
    }

    bool init() override
    {
        if (CDCLocker::locked())
        {
            return false;
        }

        _passThroughState = true;
        CDCLocker::lock();
        return true;
    }

    bool deInit() override
    {
        _passThroughState = false;
        CDCLocker::unlock();
        return true;
    }

    bool uartRead(uint8_t& value) override
    {
        if (Board::UART::read(HW_UART_CHANNEL_TOUCHSCREEN, value))
        {
            Board::IO::indicators::indicateTraffic(Board::IO::indicators::source_t::UART,
                                                   Board::IO::indicators::direction_t::INCOMING);

            return true;
        }

        return false;
    }

    bool uartWrite(uint8_t value) override
    {
        if (Board::UART::write(HW_UART_CHANNEL_TOUCHSCREEN, value))
        {
            Board::IO::indicators::indicateTraffic(Board::IO::indicators::source_t::UART,
                                                   Board::IO::indicators::direction_t::OUTGOING);

            return true;
        }

        return false;
    }

    bool cdcRead(uint8_t* buffer, size_t& size, const size_t maxSize) override
    {
        if (Board::USB::readCDC(buffer, size, maxSize))
        {
            Board::IO::indicators::indicateTraffic(Board::IO::indicators::source_t::USB,
                                                   Board::IO::indicators::direction_t::INCOMING);

            return true;
        }

        return false;
    }

    bool cdcWrite(uint8_t* buffer, size_t size) override
    {
        if (Board::USB::writeCDC(buffer, size))
        {
            Board::IO::indicators::indicateTraffic(Board::IO::indicators::source_t::USB,
                                                   Board::IO::indicators::direction_t::OUTGOING);

            return true;
        }

        return false;
    }

    void onCDCsetLineEncoding(uint32_t baudRate)
    {
        if (_passThroughState)
        {
            Board::UART::init(HW_UART_CHANNEL_TOUCHSCREEN, baudRate, true);
        }
    }

    bool allocated(IO::Common::Allocatable::interface_t interface) override
    {
        return CDCLocker::locked();
    }

    private:
    bool _passThroughState = false;
} _hwaCDCPassthrough;
#else
class HWACDCPassthroughStub : public System::Builder::HWA::IO::CDCPassthrough
{
    public:
    HWACDCPassthroughStub() = default;

    bool supported() override
    {
        return false;
    }

    bool init() override
    {
        return false;
    }

    bool deInit() override
    {
        return false;
    }

    bool uartRead(uint8_t& value) override
    {
        return false;
    }

    bool uartWrite(uint8_t value) override
    {
        return false;
    }

    bool cdcRead(uint8_t* buffer, size_t& size, const size_t maxSize) override
    {
        return false;
    }

    bool cdcWrite(uint8_t* buffer, size_t size) override
    {
        return false;
    }

    bool allocated(IO::Common::Allocatable::interface_t interface) override
    {
        return false;
    }
} _hwaCDCPassthrough;
#endif

#ifdef HW_SUPPORT_DISPLAY
class HWADisplay : public System::Builder::HWA::IO::Display
{
    public:
    HWADisplay() = default;

    bool init() override
    {
        // for i2c, consider ALREADY_INIT status a success
        return Board::I2C::init(HW_I2C_CHANNEL_DISPLAY, Board::I2C::clockSpeed_t::S400K) != Board::initStatus_t::ERROR;
    }

    bool write(uint8_t address, uint8_t* buffer, size_t size) override
    {
        return Board::I2C::write(HW_I2C_CHANNEL_DISPLAY, address, buffer, size);
    }

    bool deviceAvailable(uint8_t address) override
    {
        return Board::I2C::deviceAvailable(HW_I2C_CHANNEL_DISPLAY, address);
    }
} _hwaDisplay;
#else
class HWADisplayStub : public System::Builder::HWA::IO::Display
{
    public:
    HWADisplayStub() = default;

    bool init() override
    {
        return false;
    }

    bool write(uint8_t address, uint8_t* buffer, size_t size) override
    {
        return false;
    }

    bool deviceAvailable(uint8_t address) override
    {
        return false;
    }
} _hwaDisplay;
#endif

class HWASystem : public System::Builder::HWA::System
{
    public:
    HWASystem() = default;

    bool init() override
    {
        Board::init();
        return true;
    }

    void update() override
    {
        Board::update();

        static uint32_t lastCheckTime       = 0;
        static bool     lastConnectionState = false;

        if (core::timing::ms() - lastCheckTime > USB_CONN_CHECK_TIME)
        {
            bool newState = Board::USB::isUSBconnected();

            if (newState)
            {
                if (!lastConnectionState)
                {
                    if (_usbConnectionHandler != nullptr)
                    {
                        _usbConnectionHandler();
                    }
                }
            }

            lastConnectionState = newState;
            lastCheckTime       = core::timing::ms();
        }

        if (!CDCLocker::locked())
        {
            // nobody is using CDC, read it here to avoid lockups but ignore the data
            uint8_t dummy;
            while (Board::USB::readCDC(dummy))
            {
                ;
            }
        }
    }

    void reboot(FwSelector::fwType_t type) override
    {
        auto value = static_cast<uint8_t>(type);

        Board::bootloader::setMagicBootValue(value);
        Board::reboot();
    }

    void registerOnUSBconnectionHandler(System::Instance::usbConnectionHandler_t&& usbConnectionHandler) override
    {
        _usbConnectionHandler = std::move(usbConnectionHandler);
    }

    void disconnectUSB() override
    {
        Board::USB::deInit();
    }

    private:
    static constexpr uint32_t                USB_CONN_CHECK_TIME   = 2000;
    System::Instance::usbConnectionHandler_t _usbConnectionHandler = nullptr;
} _hwaSystem;

class HWABuilder : public ::System::Builder::HWA
{
    public:
    HWABuilder() = default;

    ::System::Builder::HWA::IO& io() override
    {
        return _hwaIO;
    }

    ::System::Builder::HWA::Protocol& protocol() override
    {
        return _hwaProtocol;
    }

    ::System::Builder::HWA::System& system() override
    {
        return _hwaSystem;
    }

    ::System::Builder::HWA::Database& database() override
    {
        return _hwaDatabase;
    }

    private:
    class HWAIO : public ::System::Builder::HWA::IO
    {
        public:
        ::System::Builder::HWA::IO::LEDs& leds() override
        {
            return _hwaLEDs;
        }

        ::System::Builder::HWA::IO::Analog& analog() override
        {
            return _hwaAnalog;
        }

        ::System::Builder::HWA::IO::Buttons& buttons() override
        {
            return _hwaButtons;
        }

        ::System::Builder::HWA::IO::Encoders& encoders() override
        {
            return _hwaEncoders;
        }

        ::System::Builder::HWA::IO::Touchscreen& touchscreen() override
        {
            return _hwaTouchscreen;
        }

        ::System::Builder::HWA::IO::CDCPassthrough& cdcPassthrough() override
        {
            return _hwaCDCPassthrough;
        }

        ::System::Builder::HWA::IO::Display& display() override
        {
            return _hwaDisplay;
        }
    } _hwaIO;

    class HWAProtocol : public ::System::Builder::HWA::Protocol
    {
        public:
        ::System::Builder::HWA::Protocol::MIDI& midi() override
        {
            return _hwaMIDI;
        }

        private:
        class HWAMIDI : public ::System::Builder::HWA::Protocol::MIDI
        {
            public:
            ::System::Builder::HWA::Protocol::MIDI::USB& usb() override
            {
                return _hwaMIDIUSB;
            }

            ::System::Builder::HWA::Protocol::MIDI::DIN& din() override
            {
                return _hwaMIDIDIN;
            }

            ::System::Builder::HWA::Protocol::MIDI::BLE& ble() override
            {
                return _hwaMIDIBLE;
            }
        } _hwaMIDI;
    } _hwaProtocol;
} _hwa;

#if defined(HW_SUPPORT_TOUCHSCREEN) && !defined(HW_USB_OVER_SERIAL)
namespace Board::USB
{
    void onCDCsetLineEncoding(uint32_t baudRate)
    {
        _hwaCDCPassthrough.onCDCsetLineEncoding(baudRate);
    }
}    // namespace Board::USB
#endif

#ifdef USE_LOGGER
class LoggerWriter : public Logger::StreamWriter
{
    public:
    LoggerWriter() = default;

    bool write(const char* message) override
    {
        return Board::USB::writeCDC((uint8_t*)&message[0], strlen(message));
    }
} _loggerWriter;
Logger logger = Logger(_loggerWriter, Logger::lineEnding_t::CRLF);
#endif

System::Builder  _builder(_hwa);
System::Instance _sys(_builder.hwa(), _builder.components());

int main()
{
    _sys.init();

    while (true)
    {
        _sys.run();
    }

    return 1;
}