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

class HWADatabase : public sys::Builder::HWA::Database
{
    public:
    HWADatabase()
    {
        MIDIDispatcher.listen(messaging::eventType_t::SYSTEM,
                              [this](const messaging::event_t& event)
                              {
                                  switch (event.systemMessage)
                                  {
                                  case messaging::systemMessage_t::RESTORE_START:
                                  {
                                      _writeToCache = true;
                                  }
                                  break;

                                  case messaging::systemMessage_t::RESTORE_END:
                                  {
                                      board::usb::deInit();
                                      board::nvm::writeCacheToFlash();
                                      board::reboot();
                                  }
                                  break;

                                  default:
                                      break;
                                  }
                              });
    }

    bool init() override
    {
        return board::nvm::init();
    }

    uint32_t size() override
    {
        return board::nvm::size();
    }

    bool clear() override
    {
#ifdef HW_SUPPORT_LED_INDICATORS
        // It's possible that LED indicators are still on since
        // this command is most likely given via USB.
        // Wait until all indicators are turned off
        core::timing::waitMs(board::io::indicators::LED_TRAFFIC_INDICATOR_TIMEOUT);
#endif

        return board::nvm::clear(0, board::nvm::size());
    }

    bool read(uint32_t address, uint32_t& value, database::Admin::sectionParameterType_t type) override
    {
        return board::nvm::read(address, value, boardParamType(type));
    }

    bool write(uint32_t address, uint32_t value, database::Admin::sectionParameterType_t type) override
    {
        return board::nvm::write(address, value, boardParamType(type), _writeToCache);
    }

    private:
    board::nvm::parameterType_t boardParamType(database::Admin::sectionParameterType_t type)
    {
        switch (type)
        {
        case database::Admin::sectionParameterType_t::WORD:
            return board::nvm::parameterType_t::WORD;

        case database::Admin::sectionParameterType_t::DWORD:
            return board::nvm::parameterType_t::DWORD;

        default:
            return board::nvm::parameterType_t::BYTE;
        }
    }

    bool _writeToCache = false;
} _hwaDatabase;

#ifdef LEDS_SUPPORTED
class HWALEDs : public sys::Builder::HWA::IO::LEDs
{
    public:
    HWALEDs() = default;

    void setState(size_t index, io::LEDs::brightness_t brightness) override
    {
        board::io::digitalOut::writeLEDstate(index, static_cast<board::io::digitalOut::ledBrightness_t>(brightness));
    }

    size_t rgbFromOutput(size_t index) override
    {
        return board::io::digitalOut::rgbFromOutput(index);
    }

    size_t rgbComponentFromRGB(size_t index, io::LEDs::rgbComponent_t component) override
    {
        return board::io::digitalOut::rgbComponentFromRGB(index,
                                                          static_cast<board::io::digitalOut::rgbComponent_t>(component));
    }
} _hwaLEDs;
#else
class HWALEDsStub : public sys::Builder::HWA::IO::LEDs
{
    public:
    HWALEDsStub() = default;

    void setState(size_t index, io::LEDs::brightness_t brightness) override
    {
    }

    size_t rgbFromOutput(size_t index) override
    {
        return 0;
    }

    size_t rgbComponentFromRGB(size_t index, io::LEDs::rgbComponent_t rgbComponent) override
    {
        return 0;
    }

    void registerHandler(void (*fptr)(size_t index, bool state))
    {
    }
} _hwaLEDs;
#endif

#ifdef HW_SUPPORT_ADC
class HWAAnalog : public sys::Builder::HWA::IO::Analog
{
    public:
    HWAAnalog() = default;

    bool value(size_t index, uint16_t& value) override
    {
        return board::io::analog::value(index, value);
    }
} _hwaAnalog;

#else
class HWAAnalogStub : public sys::Builder::HWA::IO::Analog
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
        if (!board::io::digitalIn::state(index, _dInReadA))
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
        if (!board::io::digitalIn::state(board::io::digitalIn::encoderComponentFromEncoder(index,
                                                                                           board::io::digitalIn::encoderComponent_t::A),
                                         _dInReadA))
        {
            return false;
        }

        if (!board::io::digitalIn::state(board::io::digitalIn::encoderComponentFromEncoder(index,
                                                                                           board::io::digitalIn::encoderComponent_t::B),
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
    board::io::digitalIn::readings_t _dInReadA;
#ifdef ENCODERS_SUPPORTED
    board::io::digitalIn::readings_t _dInReadB;
#endif
} _hwaDigitalIn;
#endif

#ifdef BUTTONS_SUPPORTED

class HWAButtons : public sys::Builder::HWA::IO::Buttons
{
    public:
    HWAButtons() = default;

    bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
    {
        return _hwaDigitalIn.buttonState(index, numberOfReadings, states);
    }

    size_t buttonToEncoderIndex(size_t index) override
    {
        return board::io::digitalIn::encoderFromInput(index);
    }
} _hwaButtons;
#else
class HWAButtonsStub : public sys::Builder::HWA::IO::Buttons
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

class HWAEncoders : public sys::Builder::HWA::IO::Encoders
{
    public:
    HWAEncoders() = default;

    bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
    {
        return _hwaDigitalIn.encoderState(index, numberOfReadings, states);
    }
} _hwaEncoders;
#else
class HWAEncodersStub : public sys::Builder::HWA::IO::Encoders
{
    public:
    HWAEncodersStub() = default;

    bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
    {
        return false;
    }
} _hwaEncoders;
#endif

class HWAMIDIUSB : public sys::Builder::HWA::Protocol::MIDI::USB
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
        return board::usb::init() != board::initStatus_t::ERROR;
    }

    bool deInit() override
    {
        return true;    // never deinit usb interface, just pretend here
    }

    bool read(MIDI::usbMIDIPacket_t& packet) override
    {
        if (board::usb::readMIDI(packet))
        {
            board::io::indicators::indicateTraffic(board::io::indicators::source_t::USB,
                                                   board::io::indicators::direction_t::INCOMING);

            return true;
        }

        return false;
    }

    bool write(MIDI::usbMIDIPacket_t& packet) override
    {
        if (board::usb::writeMIDI(packet))
        {
            board::io::indicators::indicateTraffic(board::io::indicators::source_t::USB,
                                                   board::io::indicators::direction_t::OUTGOING);

            return true;
        }

        return false;
    }
} _hwaMIDIUSB;

#ifdef HW_SUPPORT_DIN_MIDI
class HWAMIDIDIN : public sys::Builder::HWA::Protocol::MIDI::DIN
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
        return board::uart::init(HW_UART_CHANNEL_DIN, BAUDRATE) == board::initStatus_t::OK;
    }

    bool deInit() override
    {
        board::uart::deInit(HW_UART_CHANNEL_DIN);
        return true;
    }

    bool setLoopback(bool state) override
    {
        board::uart::setLoopbackState(HW_UART_CHANNEL_DIN, state);
        return true;
    }

    bool read(uint8_t& value) override
    {
        if (board::uart::read(HW_UART_CHANNEL_DIN, value))
        {
            board::io::indicators::indicateTraffic(board::io::indicators::source_t::UART,
                                                   board::io::indicators::direction_t::INCOMING);

            return true;
        }

        return false;
    }

    bool write(uint8_t& value) override
    {
        if (board::uart::write(HW_UART_CHANNEL_DIN, value))
        {
            board::io::indicators::indicateTraffic(board::io::indicators::source_t::UART,
                                                   board::io::indicators::direction_t::OUTGOING);

            return true;
        }

        return false;
    }

    bool allocated(io::common::Allocatable::interface_t interface) override
    {
        if (interface == io::common::Allocatable::interface_t::UART)
        {
            return board::uart::isInitialized(HW_UART_CHANNEL_DIN);
        }

        return false;
    }
} _hwaMIDIDIN;
#else
class HWAMIDIDINStub : public sys::Builder::HWA::Protocol::MIDI::DIN
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

    bool allocated(io::common::Allocatable::interface_t interface) override
    {
        return false;
    }
} _hwaMIDIDIN;
#endif

#ifdef HW_SUPPORT_BLE
class HWAMIDIBLE : public sys::Builder::HWA::Protocol::MIDI::BLE
{
    public:
    HWAMIDIBLE() = default;

    bool supported() override
    {
        return true;
    }

    bool init() override
    {
        return board::ble::init();
    }

    bool deInit() override
    {
        return board::ble::deInit();
    }

    bool write(MIDIlib::BLEMIDI::bleMIDIPacket_t& packet) override
    {
        board::io::indicators::indicateTraffic(board::io::indicators::source_t::BLE,
                                               board::io::indicators::direction_t::OUTGOING);

        return board::ble::midi::write(&packet.data[0], packet.size);
    }

    bool read(MIDIlib::BLEMIDI::bleMIDIPacket_t& packet) override
    {
        if (board::ble::midi::read(&packet.data[0], packet.size, packet.data.size()))
        {
            board::io::indicators::indicateTraffic(board::io::indicators::source_t::BLE,
                                                   board::io::indicators::direction_t::INCOMING);

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
class HWAMIDIBLEStub : public sys::Builder::HWA::Protocol::MIDI::BLE
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
class HWATouchscreen : public sys::Builder::HWA::IO::Touchscreen
{
    public:
    HWATouchscreen() = default;

    bool init() override
    {
        static constexpr uint32_t BAUDRATE = 38400;
        return board::uart::init(HW_UART_CHANNEL_TOUCHSCREEN, BAUDRATE) == board::initStatus_t::OK;
    }

    bool deInit() override
    {
        return board::uart::deInit(HW_UART_CHANNEL_TOUCHSCREEN);
    }

    bool write(uint8_t value) override
    {
        return board::uart::write(HW_UART_CHANNEL_TOUCHSCREEN, value);
    }

    bool read(uint8_t& value) override
    {
        return board::uart::read(HW_UART_CHANNEL_TOUCHSCREEN, value);
    }

    bool allocated(io::common::Allocatable::interface_t interface) override
    {
#ifdef HW_UART_CHANNEL_TOUCHSCREEN
        return board::uart::isInitialized(HW_UART_CHANNEL_TOUCHSCREEN);
#else
        return false;
#endif
    }
} _hwaTouchscreen;
#else
class HWATouchscreenStub : public sys::Builder::HWA::IO::Touchscreen
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

    bool allocated(io::common::Allocatable::interface_t interface) override
    {
        return false;
    }
} _hwaTouchscreen;
#endif

#if defined(HW_SUPPORT_TOUCHSCREEN) && !defined(HW_USB_OVER_SERIAL)
// USB link MCUs don't implement USB-CDC<->UART passthrough
class HWACDCPassthrough : public sys::Builder::HWA::IO::CDCPassthrough
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
        if (board::uart::read(HW_UART_CHANNEL_TOUCHSCREEN, value))
        {
            board::io::indicators::indicateTraffic(board::io::indicators::source_t::UART,
                                                   board::io::indicators::direction_t::INCOMING);

            return true;
        }

        return false;
    }

    bool uartWrite(uint8_t value) override
    {
        if (board::uart::write(HW_UART_CHANNEL_TOUCHSCREEN, value))
        {
            board::io::indicators::indicateTraffic(board::io::indicators::source_t::UART,
                                                   board::io::indicators::direction_t::OUTGOING);

            return true;
        }

        return false;
    }

    bool cdcRead(uint8_t* buffer, size_t& size, const size_t maxSize) override
    {
        if (board::usb::readCDC(buffer, size, maxSize))
        {
            board::io::indicators::indicateTraffic(board::io::indicators::source_t::USB,
                                                   board::io::indicators::direction_t::INCOMING);

            return true;
        }

        return false;
    }

    bool cdcWrite(uint8_t* buffer, size_t size) override
    {
        if (board::usb::writeCDC(buffer, size))
        {
            board::io::indicators::indicateTraffic(board::io::indicators::source_t::USB,
                                                   board::io::indicators::direction_t::OUTGOING);

            return true;
        }

        return false;
    }

    void onCDCsetLineEncoding(uint32_t baudRate)
    {
        if (_passThroughState)
        {
            board::uart::init(HW_UART_CHANNEL_TOUCHSCREEN, baudRate, true);
        }
    }

    bool allocated(io::common::Allocatable::interface_t interface) override
    {
        return CDCLocker::locked();
    }

    private:
    bool _passThroughState = false;
} _hwaCDCPassthrough;
#else
class HWACDCPassthroughStub : public sys::Builder::HWA::IO::CDCPassthrough
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

    bool allocated(io::common::Allocatable::interface_t interface) override
    {
        return false;
    }
} _hwaCDCPassthrough;
#endif

#ifdef HW_SUPPORT_DISPLAY
class HWADisplay : public sys::Builder::HWA::IO::Display
{
    public:
    HWADisplay() = default;

    bool init() override
    {
        // for i2c, consider ALREADY_INIT status a success
        return board::i2c::init(HW_I2C_CHANNEL_DISPLAY, board::i2c::clockSpeed_t::S400K) != board::initStatus_t::ERROR;
    }

    bool write(uint8_t address, uint8_t* buffer, size_t size) override
    {
        return board::i2c::write(HW_I2C_CHANNEL_DISPLAY, address, buffer, size);
    }

    bool deviceAvailable(uint8_t address) override
    {
        return board::i2c::deviceAvailable(HW_I2C_CHANNEL_DISPLAY, address);
    }
} _hwaDisplay;
#else
class HWADisplayStub : public sys::Builder::HWA::IO::Display
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

class HWASystem : public sys::Builder::HWA::System
{
    public:
    HWASystem()
    {
        MIDIDispatcher.listen(messaging::eventType_t::SYSTEM,
                              [](const messaging::event_t& event)
                              {
                                  switch (event.systemMessage)
                                  {
                                  case messaging::systemMessage_t::FACTORY_RESET_START:
                                  {
                                      board::usb::deInit();
                                  }
                                  break;

                                  case messaging::systemMessage_t::FACTORY_RESET_END:
                                  {
                                      board::reboot();
                                  }
                                  break;

                                  default:
                                      break;
                                  }
                              });
    }

    bool init() override
    {
        board::init();
        return true;
    }

    void update() override
    {
        board::update();

        static uint32_t lastCheckTime       = 0;
        static bool     lastConnectionState = false;

        if (core::timing::ms() - lastCheckTime > USB_CONN_CHECK_TIME)
        {
            bool newState = board::usb::isUSBconnected();

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
            while (board::usb::readCDC(dummy))
            {
                ;
            }
        }
    }

    void reboot(FwSelector::fwType_t type) override
    {
        auto value = static_cast<uint8_t>(type);

        board::bootloader::setMagicBootValue(value);
        board::reboot();
    }

    void registerOnUSBconnectionHandler(sys::Instance::usbConnectionHandler_t&& usbConnectionHandler) override
    {
        _usbConnectionHandler = std::move(usbConnectionHandler);
    }

    private:
    static constexpr uint32_t             USB_CONN_CHECK_TIME   = 2000;
    sys::Instance::usbConnectionHandler_t _usbConnectionHandler = nullptr;
} _hwaSystem;

class HWABuilder : public ::sys::Builder::HWA
{
    public:
    HWABuilder() = default;

    ::sys::Builder::HWA::IO& io() override
    {
        return _hwaIO;
    }

    ::sys::Builder::HWA::Protocol& protocol() override
    {
        return _hwaProtocol;
    }

    ::sys::Builder::HWA::System& system() override
    {
        return _hwaSystem;
    }

    ::sys::Builder::HWA::Database& database() override
    {
        return _hwaDatabase;
    }

    private:
    class HWAIO : public ::sys::Builder::HWA::IO
    {
        public:
        ::sys::Builder::HWA::IO::LEDs& leds() override
        {
            return _hwaLEDs;
        }

        ::sys::Builder::HWA::IO::Analog& analog() override
        {
            return _hwaAnalog;
        }

        ::sys::Builder::HWA::IO::Buttons& buttons() override
        {
            return _hwaButtons;
        }

        ::sys::Builder::HWA::IO::Encoders& encoders() override
        {
            return _hwaEncoders;
        }

        ::sys::Builder::HWA::IO::Touchscreen& touchscreen() override
        {
            return _hwaTouchscreen;
        }

        ::sys::Builder::HWA::IO::CDCPassthrough& cdcPassthrough() override
        {
            return _hwaCDCPassthrough;
        }

        ::sys::Builder::HWA::IO::Display& display() override
        {
            return _hwaDisplay;
        }
    } _hwaIO;

    class HWAProtocol : public ::sys::Builder::HWA::Protocol
    {
        public:
        ::sys::Builder::HWA::Protocol::MIDI& midi() override
        {
            return _hwaMIDI;
        }

        private:
        class HWAMIDI : public ::sys::Builder::HWA::Protocol::MIDI
        {
            public:
            ::sys::Builder::HWA::Protocol::MIDI::USB& usb() override
            {
                return _hwaMIDIUSB;
            }

            ::sys::Builder::HWA::Protocol::MIDI::DIN& din() override
            {
                return _hwaMIDIDIN;
            }

            ::sys::Builder::HWA::Protocol::MIDI::BLE& ble() override
            {
                return _hwaMIDIBLE;
            }
        } _hwaMIDI;
    } _hwaProtocol;
} _hwa;

#if defined(HW_SUPPORT_TOUCHSCREEN) && !defined(HW_USB_OVER_SERIAL)
namespace board::usb
{
    void onCDCsetLineEncoding(uint32_t baudRate)
    {
        _hwaCDCPassthrough.onCDCsetLineEncoding(baudRate);
    }
}    // namespace board::usb
#endif

#ifdef USE_LOGGER
class LoggerWriter : public Logger::StreamWriter
{
    public:
    LoggerWriter() = default;

    bool write(const char* message) override
    {
        return board::usb::writeCDC((uint8_t*)&message[0], strlen(message));
    }
} _loggerWriter;
Logger logger = Logger(_loggerWriter, Logger::lineEnding_t::CRLF);
#endif

sys::Builder  _builder(_hwa);
sys::Instance _sys(_builder.hwa(), _builder.components());

int main()
{
    _sys.init();

    while (true)
    {
        _sys.run();
    }

    return 1;
}