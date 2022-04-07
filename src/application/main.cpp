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

#include "core/src/general/Timing.h"
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
        return Board::NVM::clear(0, Board::NVM::size());
    }

    bool read(uint32_t address, int32_t& value, Database::Instance::sectionParameterType_t type) override
    {
        return Board::NVM::read(address, value, boardParamType(type));
    }

    bool write(uint32_t address, int32_t value, Database::Instance::sectionParameterType_t type) override
    {
        return Board::NVM::write(address, value, boardParamType(type));
    }

    private:
    Board::NVM::parameterType_t boardParamType(Database::Instance::sectionParameterType_t type)
    {
        switch (type)
        {
        case Database::Instance::sectionParameterType_t::WORD:
            return Board::NVM::parameterType_t::WORD;

        case Database::Instance::sectionParameterType_t::DWORD:
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
        Board::IO::writeLEDstate(index, appToBoardBrightness(brightness));
    }

    size_t rgbIndex(size_t singleLEDindex) override
    {
        return Board::IO::rgbIndex(singleLEDindex);
    }

    size_t rgbSignalIndex(size_t rgbIndex, IO::LEDs::rgbIndex_t rgbComponent) override
    {
        Board::IO::rgbIndex_t boardRGBindex;

        switch (rgbComponent)
        {
        case IO::LEDs::rgbIndex_t::R:
        {
            boardRGBindex = Board::IO::rgbIndex_t::R;
        }
        break;

        case IO::LEDs::rgbIndex_t::G:
        {
            boardRGBindex = Board::IO::rgbIndex_t::G;
        }
        break;

        case IO::LEDs::rgbIndex_t::B:
        {
            boardRGBindex = Board::IO::rgbIndex_t::B;
        }
        break;

        default:
            return 0;
        }

        return Board::IO::rgbSignalIndex(rgbIndex, boardRGBindex);
    }

    Board::IO::ledBrightness_t appToBoardBrightness(IO::LEDs::brightness_t brightness)
    {
        switch (brightness)
        {
        case IO::LEDs::brightness_t::B25:
            return Board::IO::ledBrightness_t::B25;

        case IO::LEDs::brightness_t::B50:
            return Board::IO::ledBrightness_t::B50;

        case IO::LEDs::brightness_t::B75:
            return Board::IO::ledBrightness_t::B75;

        case IO::LEDs::brightness_t::B100:
            return Board::IO::ledBrightness_t::B100;

        default:
            return Board::IO::ledBrightness_t::OFF;
        }
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
class HWAAnalog : public System::Builder::HWA::IO::Analog
{
    public:
    HWAAnalog() = default;

    bool value(size_t index, uint16_t& value) override
    {
        return Board::IO::analogValue(index, value);
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
        if (!Board::IO::digitalInState(index, _dInReadA))
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
        if (!Board::IO::digitalInState(Board::IO::encoderSignalIndex(index, Board::IO::encoderIndex_t::A), _dInReadA))
        {
            return false;
        }

        if (!Board::IO::digitalInState(Board::IO::encoderSignalIndex(index, Board::IO::encoderIndex_t::B), _dInReadB))
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
            BIT_WRITE(states, (i * 2) + 1, (_dInReadA.readings >> i & 0x01));
            BIT_WRITE(states, i * 2, (_dInReadB.readings >> i & 0x01));
        }

        return true;
    }
#endif

    private:
    Board::IO::dInReadings_t _dInReadA;
#ifdef ENCODERS_SUPPORTED
    Board::IO::dInReadings_t _dInReadB;
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
        return Board::IO::encoderIndex(index);
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
        return true;    // already initialized
    }

    bool deInit() override
    {
        return true;    // never deinit usb interface, just pretend here
    }

    bool read(MIDI::usbMIDIPacket_t& packet) override
    {
        if (Board::USB::readMIDI(packet))
        {
            Board::IO::indicateTraffic(Board::IO::dataSource_t::USB, Board::IO::dataDirection_t::INCOMING);
            return true;
        }

        return false;
    }

    bool write(MIDI::usbMIDIPacket_t& packet) override
    {
        if (Board::USB::writeMIDI(packet))
        {
            Board::IO::indicateTraffic(Board::IO::dataSource_t::USB, Board::IO::dataDirection_t::OUTGOING);
            return true;
        }

        return false;
    }
} _hwaMIDIUSB;

#ifdef DIN_MIDI_SUPPORTED
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
        Board::UART::config_t config(UART_BAUDRATE_MIDI_STD,
                                     Board::UART::parity_t::NO,
                                     Board::UART::stopBits_t::ONE,
                                     Board::UART::type_t::RX_TX);

        return Board::UART::init(UART_CHANNEL_DIN, config) == Board::UART::initStatus_t::OK;
    }

    bool deInit() override
    {
        Board::UART::deInit(UART_CHANNEL_DIN);
        return true;
    }

    bool setLoopback(bool state) override
    {
        Board::UART::setLoopbackState(UART_CHANNEL_DIN, state);
        return true;
    }

    bool read(uint8_t& value) override
    {
        if (Board::UART::read(UART_CHANNEL_DIN, value))
        {
            Board::IO::indicateTraffic(Board::IO::dataSource_t::UART, Board::IO::dataDirection_t::INCOMING);
            return true;
        }

        return false;
    }

    bool write(uint8_t& value) override
    {
        if (Board::UART::write(UART_CHANNEL_DIN, value))
        {
            Board::IO::indicateTraffic(Board::IO::dataSource_t::UART, Board::IO::dataDirection_t::OUTGOING);
            return true;
        }

        return false;
    }

    bool allocated(IO::Common::interface_t interface) override
    {
        if (interface == IO::Common::interface_t::UART)
        {
            return Board::UART::isInitialized(UART_CHANNEL_DIN);
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

    bool allocated(IO::Common::interface_t interface) override
    {
        return false;
    }
} _hwaMIDIDIN;
#endif

#ifdef BLE_SUPPORTED
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
        Board::IO::indicateTraffic(Board::IO::dataSource_t::BLE, Board::IO::dataDirection_t::OUTGOING);
        return Board::BLE::MIDI::write(&packet.data[0], packet.size);
    }

    bool read(MIDIlib::BLEMIDI::bleMIDIPacket_t& packet) override
    {
        if (Board::BLE::MIDI::read(&packet.data[0], packet.size, packet.data.size()))
        {
            Board::IO::indicateTraffic(Board::IO::dataSource_t::BLE, Board::IO::dataDirection_t::INCOMING);
            return true;
        }

        return false;
    }

    uint32_t time() override
    {
        return core::timing::currentRunTimeMs();
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

#ifdef DMX_SUPPORTED
class HWADMX : public System::Builder::HWA::Protocol::DMX
{
    public:
    HWADMX() = default;

    bool init() override
    {
        Board::UART::config_t config(250000,
                                     Board::UART::parity_t::NO,
                                     Board::UART::stopBits_t::TWO,
                                     Board::UART::type_t::TX,
                                     true);

        if (Board::UART::init(UART_CHANNEL_DMX, config) == Board::UART::initStatus_t::OK)
        {
            CDCLocker::lock();
            return true;
        }

        return false;
    }

    bool deInit() override
    {
        if (Board::UART::deInit(UART_CHANNEL_DMX))
        {
            CDCLocker::unlock();
            return true;
        }

        return false;
    }

    bool readUSB(DMXUSBWidget::usbReadBuffer_t& buffer, size_t& size) override
    {
        size_t bufferSize = 0;

        if (Board::USB::readCDC(&buffer[0], bufferSize, buffer.size()))
        {
            size = bufferSize;
            Board::IO::indicateTraffic(Board::IO::dataSource_t::USB, Board::IO::dataDirection_t::INCOMING);
            return true;
        }

        return false;
    }

    bool writeUSB(uint8_t* buffer, size_t size) override
    {
        if (Board::USB::writeCDC(buffer, size))
        {
            Board::IO::indicateTraffic(Board::IO::dataSource_t::USB, Board::IO::dataDirection_t::OUTGOING);
            return true;
        }

        return false;
    }

    void setBuffer(DMXUSBWidget::dmxBuffer_t& buffer) override
    {
        Board::IO::indicateTraffic(Board::IO::dataSource_t::UART, Board::IO::dataDirection_t::OUTGOING);
        Board::UART::setDMXBuffer(&buffer[0]);
    }

    bool uniqueID(Protocol::DMX::uniqueID_t& uniqueID) override
    {
        Board::uniqueID(uniqueID);
        return true;
    }

    bool allocated(IO::Common::interface_t interface) override
    {
        if (interface == IO::Common::interface_t::UART)
        {
#ifdef UART_CHANNEL_DMX
            return Board::UART::isInitialized(UART_CHANNEL_DMX);
#else
            return false;
#endif
        }
        else
        {
            return CDCLocker::locked();
        }
    }
} _hwaDMX;
#else
class HWADMXStub : public System::Builder::HWA::Protocol::DMX
{
    public:
    HWADMXStub() = default;

    bool init() override
    {
        return false;
    }

    bool deInit() override
    {
        return false;
    }

    bool readUSB(DMXUSBWidget::usbReadBuffer_t& buffer, size_t& size) override
    {
        return false;
    }

    bool writeUSB(uint8_t* buffer, size_t size) override
    {
        return false;
    }

    void setBuffer(DMXUSBWidget::dmxBuffer_t& buffer) override
    {
    }

    bool uniqueID(Protocol::DMX::uniqueID_t& uniqueID) override
    {
        return false;
    }

    bool allocated(IO::Common::interface_t interface) override
    {
        return false;
    }
} _hwaDMX;
#endif

#ifdef TOUCHSCREEN_SUPPORTED
class HWATouchscreen : public System::Builder::HWA::IO::Touchscreen
{
    public:
    HWATouchscreen() = default;

    bool init() override
    {
        Board::UART::config_t config(UART_BAUDRATE_TOUCHSCREEN,
                                     Board::UART::parity_t::NO,
                                     Board::UART::stopBits_t::ONE,
                                     Board::UART::type_t::RX_TX);

        return Board::UART::init(UART_CHANNEL_TOUCHSCREEN, config) == Board::UART::initStatus_t::OK;
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

    bool allocated(IO::Common::interface_t interface) override
    {
#ifdef UART_CHANNEL_TOUCHSCREEN
        return Board::UART::isInitialized(UART_CHANNEL_TOUCHSCREEN);
#else
        return false;
#endif
    }
} _hwaTouchscreen;

class HWACDCPassthrough : public System::Builder::HWA::IO::CDCPassthrough
{
    public:
    HWACDCPassthrough() = default;

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
        if (Board::UART::read(UART_CHANNEL_TOUCHSCREEN, value))
        {
            Board::IO::indicateTraffic(Board::IO::dataSource_t::UART, Board::IO::dataDirection_t::INCOMING);
            return true;
        }

        return false;
    }

    bool uartWrite(uint8_t value) override
    {
        if (Board::UART::write(UART_CHANNEL_TOUCHSCREEN, value))
        {
            Board::IO::indicateTraffic(Board::IO::dataSource_t::UART, Board::IO::dataDirection_t::OUTGOING);
            return true;
        }

        return false;
    }

    bool cdcRead(uint8_t* buffer, size_t& size, const size_t maxSize) override
    {
        if (Board::USB::readCDC(buffer, size, maxSize))
        {
            Board::IO::indicateTraffic(Board::IO::dataSource_t::USB, Board::IO::dataDirection_t::INCOMING);
            return true;
        }

        return false;
    }

    bool cdcWrite(uint8_t* buffer, size_t size) override
    {
        if (Board::USB::writeCDC(buffer, size))
        {
            Board::IO::indicateTraffic(Board::IO::dataSource_t::USB, Board::IO::dataDirection_t::OUTGOING);
            return true;
        }

        return false;
    }

    void onCDCsetLineEncoding(uint32_t baudRate)
    {
        if (_passThroughState)
        {
            Board::UART::config_t config(baudRate,
                                         Board::UART::parity_t::NO,
                                         Board::UART::stopBits_t::ONE,
                                         Board::UART::type_t::RX_TX);

            Board::UART::init(UART_CHANNEL_TOUCHSCREEN, config, true);
        }
    }

    bool allocated(IO::Common::interface_t interface) override
    {
        return CDCLocker::locked();
    }

    private:
    bool _passThroughState = false;
} _hwaCDCPassthrough;
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

    bool allocated(IO::Common::interface_t interface) override
    {
        return false;
    }
} _hwaTouchscreen;

class HWACDCPassthroughStub : public System::Builder::HWA::IO::CDCPassthrough
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

    bool allocated(IO::Common::interface_t interface) override
    {
        return false;
    }
} _hwaCDCPassthrough;
#endif

#ifdef I2C_SUPPORTED
class HWAI2C : public System::Builder::HWA::IO::I2C
{
    public:
    HWAI2C() = default;

    bool init() override
    {
        return Board::I2C::init(I2C_CHANNEL, Board::I2C::clockSpeed_t::S400K);
    }

    bool write(uint8_t address, uint8_t* buffer, size_t size) override
    {
        return Board::I2C::write(I2C_CHANNEL, address, buffer, size);
    }

    bool deviceAvailable(uint8_t address) override
    {
        return Board::I2C::deviceAvailable(I2C_CHANNEL, address);
    }
} _hwaI2C;
#else
class HWAI2CStub : public System::Builder::HWA::IO::I2C
{
    public:
    HWAI2CStub() = default;

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
} _hwaI2C;
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

        if (core::timing::currentRunTimeMs() - lastCheckTime > USB_CONN_CHECK_TIME)
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
            lastCheckTime       = core::timing::currentRunTimeMs();
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

        ::System::Builder::HWA::IO::I2C& i2c() override
        {
            return _hwaI2C;
        }
    } _hwaIO;

    class HWAProtocol : public ::System::Builder::HWA::Protocol
    {
        public:
        ::System::Builder::HWA::Protocol::MIDI& midi() override
        {
            return _hwaMIDI;
        }

        ::System::Builder::HWA::Protocol::DMX& dmx() override
        {
            return _hwaDMX;
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

#ifdef TOUCHSCREEN_SUPPORTED
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