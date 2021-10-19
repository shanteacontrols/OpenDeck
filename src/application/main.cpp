/*

Copyright 2015-2021 Igor Petrovic

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

#include "core/src/general/Timing.h"
#include "board/Board.h"
#include "system/System.h"

class CDCUser
{
    public:
    virtual bool isUsed() = 0;
};

class StorageAccess : public LESSDB::StorageAccess
{
    public:
    StorageAccess() = default;

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
        return Board::NVM::clear(0, Board::NVM::size());
    }

    bool read(uint32_t address, int32_t& value, LESSDB::sectionParameterType_t type) override
    {
        return Board::NVM::read(address, value, boardParamType(type));
    }

    bool write(uint32_t address, int32_t value, LESSDB::sectionParameterType_t type) override
    {
        return Board::NVM::write(address, value, boardParamType(type));
    }

    size_t paramUsage(LESSDB::sectionParameterType_t type) override
    {
        return Board::NVM::paramUsage(boardParamType(type));
    }

    private:
    Board::NVM::parameterType_t boardParamType(LESSDB::sectionParameterType_t type)
    {
        switch (type)
        {
        case LESSDB::sectionParameterType_t::word:
            return Board::NVM::parameterType_t::word;

        case LESSDB::sectionParameterType_t::dword:
            return Board::NVM::parameterType_t::dword;

        default:
            return Board::NVM::parameterType_t::byte;
        }
    }
} _storageAccess;

#ifdef LEDS_SUPPORTED
class HWALEDs : public System::HWA::IO::LEDs
{
    public:
    HWALEDs() = default;

    bool supported() override
    {
        return true;
    }

    void setState(size_t index, IO::LEDs::brightness_t brightness) override
    {
        Board::io::writeLEDstate(index, appToBoardBrightness(brightness));
    }

    size_t rgbIndex(size_t singleLEDindex) override
    {
#if MAX_NUMBER_OF_LEDS > 0
        return Board::io::rgbIndex(singleLEDindex);
#else
        return 0;
#endif
    }

    size_t rgbSignalIndex(size_t rgbIndex, IO::LEDs::rgbIndex_t rgbComponent) override
    {
#if MAX_NUMBER_OF_LEDS > 0
        Board::io::rgbIndex_t boardRGBindex;

        switch (rgbComponent)
        {
        case IO::LEDs::rgbIndex_t::r:
        {
            boardRGBindex = Board::io::rgbIndex_t::r;
        }
        break;

        case IO::LEDs::rgbIndex_t::g:
        {
            boardRGBindex = Board::io::rgbIndex_t::g;
        }
        break;

        case IO::LEDs::rgbIndex_t::b:
        {
            boardRGBindex = Board::io::rgbIndex_t::b;
        }
        break;

        default:
            return 0;
        }

        return Board::io::rgbSignalIndex(rgbIndex, boardRGBindex);
#else
        return 0;
#endif
    }

    Board::io::ledBrightness_t appToBoardBrightness(IO::LEDs::brightness_t brightness)
    {
        switch (brightness)
        {
        case IO::LEDs::brightness_t::b25:
            return Board::io::ledBrightness_t::b25;

        case IO::LEDs::brightness_t::b50:
            return Board::io::ledBrightness_t::b50;

        case IO::LEDs::brightness_t::b75:
            return Board::io::ledBrightness_t::b75;

        case IO::LEDs::brightness_t::b100:
            return Board::io::ledBrightness_t::b100;

        default:
            return Board::io::ledBrightness_t::bOff;
        }
    }
} _hwaLEDs;
#else
class HWALEDsStub : public System::HWA::IO::LEDs
{
    public:
    HWALEDsStub() {}

    bool supported() override
    {
        return false;
    }

    void setState(size_t index, bool state) override
    {
    }

    size_t rgbIndex(size_t singleLEDindex) override
    {
        return 0;
    }

    size_t rgbSignalIndex(size_t rgbIndex, IO::LEDs::rgbIndex_t rgbComponent) override
    {
        return 0;
    }

    void registerHandler(void (*fptr)(size_t index, bool state))
    {
    }
} _hwaLEDs;
#endif

#ifdef ANALOG_SUPPORTED
class HWAAnalog : public System::HWA::IO::Analog
{
    public:
    HWAAnalog() = default;

    bool supported() override
    {
        return true;
    }

    bool value(size_t index, uint16_t& value) override
    {
        return Board::io::analogValue(index, value);
    }
} _hwaAnalog;

#else
class HWAAnalogStub : public System::HWA::IO::Analog
{
    public:
    HWAAnalogStub() {}

    bool supported() override
    {
        return false;
    }

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
        if (!Board::io::digitalInState(index, dInReadA))
            return false;

        numberOfReadings = dInReadA.count;
        states           = dInReadA.readings;

        return true;
    }
#endif

#ifdef ENCODERS_SUPPORTED
    bool encoderState(size_t index, uint8_t& numberOfReadings, uint32_t& states)
    {
        if (!Board::io::digitalInState(Board::io::encoderSignalIndex(index, Board::io::encoderIndex_t::a), dInReadA))
            return false;

        if (!Board::io::digitalInState(Board::io::encoderSignalIndex(index, Board::io::encoderIndex_t::b), dInReadB))
            return false;

        numberOfReadings = dInReadA.count > dInReadB.count ? dInReadA.count : dInReadB.count;

        // construct encoder pair readings
        // encoder signal is made of A and B signals
        // take each bit of A signal and B signal and append it to states variable in order
        // latest encoder readings should be in LSB bits

        for (uint8_t i = 0; i < numberOfReadings; i++)
        {
            BIT_WRITE(states, (i * 2) + 1, (dInReadA.readings >> i & 0x01));
            BIT_WRITE(states, i * 2, (dInReadB.readings >> i & 0x01));
        }

        return true;
    }
#endif

    private:
    Board::io::dInReadings_t dInReadA;
#ifdef ENCODERS_SUPPORTED
    Board::io::dInReadings_t dInReadB;
#endif
} _hwaDigitalIn;
#endif

#ifdef BUTTONS_SUPPORTED

class HWAButtons : public System::HWA::IO::Buttons
{
    public:
    HWAButtons() = default;

    bool supported() override
    {
        return true;
    }

    bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
    {
        return _hwaDigitalIn.buttonState(index, numberOfReadings, states);
    }

    size_t buttonToEncoderIndex(size_t index) override
    {
        return Board::io::encoderIndex(index);
    }
} _hwaButtons;
#else
class HWAButtonsStub : public System::HWA::IO::Buttons
{
    public:
    HWAButtonsStub() {}

    bool supported() override
    {
        return false;
    }

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

class HWAEncoders : public System::HWA::IO::Encoders
{
    public:
    HWAEncoders() = default;

    bool supported() override
    {
        return true;
    }

    bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
    {
        return _hwaDigitalIn.encoderState(index, numberOfReadings, states);
    }
} _hwaEncoders;
#else
class HWAEncodersStub : public System::HWA::IO::Encoders
{
    public:
    HWAEncodersStub() {}

    bool supported() override
    {
        return false;
    }

    bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
    {
        return false;
    }
} _hwaEncoders;
#endif

class HWAMIDI : public System::HWA::Protocol::MIDI
{
    public:
    HWAMIDI() = default;

    bool dinSupported() override
    {
#ifdef DIN_MIDI_SUPPORTED
        return true;
#else
        return false;
#endif
    }

    bool init(::MIDI::interface_t interface) override
    {
        if (interface == ::MIDI::interface_t::usb)
            return true;    // already initialized

#ifdef DIN_MIDI_SUPPORTED
        Board::UART::config_t config(UART_BAUDRATE_MIDI_STD,
                                     Board::UART::parity_t::no,
                                     Board::UART::stopBits_t::one,
                                     Board::UART::type_t::rxTx);

        return Board::UART::init(UART_CHANNEL_DIN, config) == Board::UART::initStatus_t::ok;

#else
        return false;
#endif
    }

    bool deInit(::MIDI::interface_t interface) override
    {
        if (interface == ::MIDI::interface_t::usb)
            return true;    // never deinit usb interface, just pretend here

#ifdef DIN_MIDI_SUPPORTED
        Board::UART::deInit(UART_CHANNEL_DIN);
        return true;
#else
        return false;
#endif
    }

    bool setDINLoopback(bool state)
    {
#ifdef DIN_MIDI_SUPPORTED
        Board::UART::setLoopbackState(UART_CHANNEL_DIN, state);
        return true;
#else
        return true;
#endif
    }

    bool dinRead(uint8_t& value) override
    {
#ifdef DIN_MIDI_SUPPORTED
        if (Board::UART::read(UART_CHANNEL_DIN, value))
        {
            Board::io::indicateTraffic(Board::io::dataSource_t::uart, Board::io::dataDirection_t::incoming);
            return true;
        }

        return false;
#else
        return false;
#endif
    }

    bool dinWrite(uint8_t value) override
    {
#ifdef DIN_MIDI_SUPPORTED
        if (Board::UART::write(UART_CHANNEL_DIN, value))
        {
            Board::io::indicateTraffic(Board::io::dataSource_t::uart, Board::io::dataDirection_t::outgoing);
            return true;
        }

        return false;
#else
        return false;
#endif
    }

    bool usbRead(::MIDI::USBMIDIpacket_t& USBMIDIpacket) override
    {
        if (Board::USB::readMIDI(USBMIDIpacket))
        {
            Board::io::indicateTraffic(Board::io::dataSource_t::usb, Board::io::dataDirection_t::incoming);
            return true;
        }

        return false;
    }

    bool usbWrite(::MIDI::USBMIDIpacket_t& USBMIDIpacket) override
    {
        if (Board::USB::writeMIDI(USBMIDIpacket))
        {
            Board::io::indicateTraffic(Board::io::dataSource_t::usb, Board::io::dataDirection_t::outgoing);
            return true;
        }

        return false;
    }
} _hwaMIDI;

#ifdef DMX_SUPPORTED
class HWADMX : public System::HWA::Protocol::DMX, public CDCUser
{
    public:
    HWADMX() = default;

    bool supported() override
    {
        return true;
    }

    bool init() override
    {
        Board::UART::config_t config(250000,
                                     Board::UART::parity_t::no,
                                     Board::UART::stopBits_t::two,
                                     Board::UART::type_t::tx,
                                     true);

        if (Board::UART::init(UART_CHANNEL_DMX, config) == Board::UART::initStatus_t::ok)
        {
            _isUsed = true;
            return true;
        }

        return false;
    }

    bool deInit() override
    {
        if (Board::UART::deInit(UART_CHANNEL_DMX))
        {
            _isUsed = false;
            return true;
        }

        return false;
    }

    bool readUSB(uint8_t* buffer, size_t& size, const size_t maxSize) override
    {
        size_t bufferSize = 0;

        if (Board::USB::readCDC(buffer, bufferSize, maxSize))
        {
            size = bufferSize;
            Board::io::indicateTraffic(Board::io::dataSource_t::usb, Board::io::dataDirection_t::incoming);
            return true;
        }

        return false;
    }

    bool writeUSB(uint8_t* buffer, size_t size) override
    {
        if (Board::USB::writeCDC(buffer, size))
        {
            Board::io::indicateTraffic(Board::io::dataSource_t::usb, Board::io::dataDirection_t::outgoing);
            return true;
        }

        return false;
    }

    bool updateChannel(uint16_t channel, uint8_t value) override
    {
        Board::UART::setDMXChannelValue(channel, value);
        Board::io::indicateTraffic(Board::io::dataSource_t::uart, Board::io::dataDirection_t::outgoing);
        return true;
    }

    void packetComplete() override
    {
        // todo
    }

    bool isUsed() override
    {
        return _isUsed;
    }

    private:
    bool _isUsed = false;
} _hwaDMX;
#else
class HWADMXStub : public System::HWA::Protocol::DMX, public CDCUser
{
    public:
    HWADMXStub() = default;

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

    bool readUSB(uint8_t* buffer, size_t& size, const size_t maxSize) override
    {
        return false;
    }

    bool writeUSB(uint8_t* buffer, size_t size) override
    {
        return false;
    }

    bool updateChannel(uint16_t channel, uint8_t value) override
    {
        return false;
    }

    void packetComplete() override
    {
    }

    bool isUsed() override
    {
        return false;
    }
} _hwaDMX;
#endif

#ifdef TOUCHSCREEN_SUPPORTED
class HWATouchscreen : public System::HWA::IO::Touchscreen
{
    public:
    HWATouchscreen() {}

    bool supported() override
    {
        return true;
    }

    bool init() override
    {
        Board::UART::config_t config(UART_BAUDRATE_TOUCHSCREEN,
                                     Board::UART::parity_t::no,
                                     Board::UART::stopBits_t::one,
                                     Board::UART::type_t::rxTx);

        return Board::UART::init(UART_CHANNEL_TOUCHSCREEN, config) == Board::UART::initStatus_t::ok;
    }

    bool deInit() override
    {
        return Board::UART::deInit(UART_CHANNEL_TOUCHSCREEN);
    }

    bool write(uint8_t value) override
    {
        return Board::UART::write(UART_CHANNEL_TOUCHSCREEN, value);
    }

    bool read(uint8_t& value) override
    {
        return Board::UART::read(UART_CHANNEL_TOUCHSCREEN, value);
    }
} _hwaTouchscreen;

class HWACDCPassthrough : public System::HWA::IO::CDCPassthrough, public CDCUser
{
    public:
    HWACDCPassthrough() = default;

    bool init() override
    {
        _passThroughState = true;
        return true;
    }

    bool deInit() override
    {
        _passThroughState = false;
        return true;
    }

    bool uartRead(uint8_t& value) override
    {
        if (Board::UART::read(UART_CHANNEL_TOUCHSCREEN, value))
        {
            Board::io::indicateTraffic(Board::io::dataSource_t::uart, Board::io::dataDirection_t::incoming);
            return true;
        }

        return false;
    }

    bool uartWrite(uint8_t value) override
    {
        if (Board::UART::write(UART_CHANNEL_TOUCHSCREEN, value))
        {
            Board::io::indicateTraffic(Board::io::dataSource_t::uart, Board::io::dataDirection_t::outgoing);
            return true;
        }

        return false;
    }

    bool cdcRead(uint8_t* buffer, size_t& size, const size_t maxSize) override
    {
        if (Board::USB::readCDC(buffer, size, maxSize))
        {
            Board::io::indicateTraffic(Board::io::dataSource_t::usb, Board::io::dataDirection_t::incoming);
            return true;
        }

        return false;
    }

    bool cdcWrite(uint8_t* buffer, size_t size) override
    {
        if (Board::USB::writeCDC(buffer, size))
        {
            Board::io::indicateTraffic(Board::io::dataSource_t::usb, Board::io::dataDirection_t::outgoing);
            return true;
        }

        return false;
    }

    void onCDCsetLineEncoding(uint32_t baudRate)
    {
        if (_passThroughState)
        {
            Board::UART::config_t config(baudRate,
                                         Board::UART::parity_t::no,
                                         Board::UART::stopBits_t::one,
                                         Board::UART::type_t::rxTx);

            Board::UART::init(UART_CHANNEL_TOUCHSCREEN, config, true);
        }
    }

    bool isUsed() override
    {
        return _passThroughState;
    }

    private:
    bool _passThroughState = false;
} _hwaCDCPassthrough;
#else
class HWATouchscreenStub : public System::HWA::IO::Touchscreen
{
    public:
    HWATouchscreenStub() {}

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

    bool write(uint8_t value) override
    {
        return false;
    }

    bool read(uint8_t& value) override
    {
        return false;
    }
} _hwaTouchscreen;

class HWACDCPassthroughStub : public System::HWA::IO::CDCPassthrough, public CDCUser
{
    public:
    HWACDCPassthroughStub() = default;

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

    bool isUsed() override
    {
        return false;
    }

    private:
} _hwaCDCPassthrough;
#endif

#ifdef DISPLAY_SUPPORTED
class Display : public System::HWA::IO::Display
{
    public:
    Display() {}

    bool supported() override
    {
        return true;
    }

    bool init() override
    {
        return Board::I2C::init(I2C_CHANNEL_DISPLAY, Board::I2C::clockSpeed_t::_1kHz);
    }

    bool deInit() override
    {
        return Board::I2C::deInit(I2C_CHANNEL_DISPLAY);
    }

    bool write(uint8_t address, uint8_t* buffer, size_t size) override
    {
        return Board::I2C::write(I2C_CHANNEL_DISPLAY, address, buffer, size);
    }
} _hwaDisplay;
#else
class DisplayStub : public System::HWA::IO::Display
{
    public:
    DisplayStub() = default;

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

    bool write(uint8_t address, uint8_t* buffer, size_t size) override
    {
        return false;
    }
} _hwaDisplay;
#endif

class HWASystem : public System::HWA
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
        static uint32_t lastCheckTime       = 0;
        static bool     lastConnectionState = false;
        bool            readCDC             = true;

        if (core::timing::currentRunTimeMs() - lastCheckTime > USB_CONN_CHECK_TIME)
        {
            bool newState = Board::USB::isUSBconnected();

            if (newState)
            {
                if (!lastConnectionState)
                {
                    if (_usbConnectionHandler != nullptr)
                        _usbConnectionHandler();
                }
            }

            lastConnectionState = newState;
            lastCheckTime       = core::timing::currentRunTimeMs();
        }

        for (size_t i = 0; i < _cdcUserCount; i++)
        {
            if (_cdcUser[i]->isUsed())
            {
                readCDC = false;
                break;
            }
        }

        if (readCDC)
        {
            // nobody is using CDC, read it here to avoid lockups but ignore the data
            uint8_t dummy;
            while (Board::USB::readCDC(dummy))
                ;
        }
    }

    void reboot(FwSelector::fwType_t type) override
    {
        auto value = static_cast<uint8_t>(type);

        Board::bootloader::setMagicBootValue(value);
        Board::reboot();
    }

    void registerOnUSBconnectionHandler(System::usbConnectionHandler_t&& usbConnectionHandler) override
    {
        _usbConnectionHandler = std::move(usbConnectionHandler);
    }

    bool serialPeripheralAllocated(System::serialPeripheral_t peripheral) override
    {
#ifdef USE_UART
        switch (peripheral)
        {
        case System::serialPeripheral_t::dinMIDI:
        {
#ifdef UART_CHANNEL_DIN
            return Board::UART::isInitialized(UART_CHANNEL_DIN);
#else
            return false;
#endif
        }
        break;

        case System::serialPeripheral_t::touchscreen:
        {
#ifdef UART_CHANNEL_TOUCHSCREEN
            return Board::UART::isInitialized(UART_CHANNEL_TOUCHSCREEN);
#else
            return false;
#endif
        }
        break;

        case System::serialPeripheral_t::dmx:
        {
#ifdef UART_CHANNEL_DMX
            return Board::UART::isInitialized(UART_CHANNEL_DMX);
#else
            return false;
#endif
        }
        break;

        default:
            return false;
        }
#else
        return false;
#endif
    }

    bool uniqueID(System::uniqueID_t& uniqueID) override
    {
        Board::uniqueID(uniqueID);
        return true;
    }

    System::HWA::IO& io() override
    {
        return _hwaIO;
    }

    System::HWA::Protocol& protocol() override
    {
        return _hwaProtocol;
    }

    void addCDCUser(CDCUser& cdcUser)
    {
        if (_cdcUserCount == MAX_CDC_USERS)
            return;

        _cdcUser[_cdcUserCount++] = &cdcUser;
    }

    private:
    static constexpr uint32_t      USB_CONN_CHECK_TIME   = 2000;
    static constexpr uint32_t      MAX_CDC_USERS         = 2;
    System::usbConnectionHandler_t _usbConnectionHandler = nullptr;
    CDCUser*                       _cdcUser[MAX_CDC_USERS];
    size_t                         _cdcUserCount = 0;

    class SystemHWAIO : public System::HWA::IO
    {
        public:
        SystemHWAIO() = default;

        System::HWA::IO::LEDs& leds() override
        {
            return _hwaLEDs;
        }

        System::HWA::IO::Analog& analog() override
        {
            return _hwaAnalog;
        }

        System::HWA::IO::Buttons& buttons() override
        {
            return _hwaButtons;
        }

        System::HWA::IO::Encoders& encoders() override
        {
            return _hwaEncoders;
        }

        System::HWA::IO::Touchscreen& touchscreen() override
        {
            return _hwaTouchscreen;
        }

        System::HWA::IO::CDCPassthrough& cdcPassthrough() override
        {
            return _hwaCDCPassthrough;
        }

        System::HWA::IO::Display& display() override
        {
            return _hwaDisplay;
        }
    } _hwaIO;

    class SystemHWAProtocol : public System::HWA::Protocol
    {
        public:
        System::HWA::Protocol::MIDI& midi()
        {
            return _hwaMIDI;
        }

        System::HWA::Protocol::DMX& dmx()
        {
            return _hwaDMX;
        }
    } _hwaProtocol;
} _hwaSystem;

#ifdef TOUCHSCREEN_SUPPORTED
namespace Board
{
    namespace USB
    {
        void onCDCsetLineEncoding(uint32_t baudRate)
        {
            _hwaCDCPassthrough.onCDCsetLineEncoding(baudRate);
        }
    }    // namespace USB
}    // namespace Board
#endif

Database _database(_storageAccess,
#ifdef __AVR__
                   true
#else
                   false
#endif
);
System sys(_hwaSystem, _database);

int main()
{
    _hwaSystem.addCDCUser(_hwaDMX);
    _hwaSystem.addCDCUser(_hwaCDCPassthrough);

    sys.init();

    while (true)
    {
        sys.run();
    }

    return 1;
}