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

#include "core/MCU.h"
#include "board/Board.h"
#include "application/io/common/Common.h"
#include "application/system/System.h"
#include "application/system/Builder.h"
#ifdef APP_USE_LOGGER
#include "application/util/logger/Logger.h"
#endif

class CdcLocker
{
    public:
    CdcLocker() = default;

    static bool locked()
    {
        return isLocked;
    }

    static void lock()
    {
        isLocked = true;
    }

    static void unlock()
    {
        isLocked = false;
    }

    private:
    static bool isLocked;
};

bool CdcLocker::isLocked = false;

class HwaDatabase : public sys::Builder::HWA::Database
{
    public:
    HwaDatabase()
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
#ifdef PROJECT_TARGET_SUPPORT_LED_INDICATORS
        // It's possible that LED indicators are still on since
        // this command is most likely given via USB.
        // Wait until all indicators are turned off
        core::mcu::timing::waitMs(board::io::indicators::LED_TRAFFIC_INDICATOR_TIMEOUT);
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

    bool initializeDatabase() override
    {
        return PROJECT_MCU_DATABASE_INIT_DATA;
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
} hwaDatabase;

#ifdef LEDS_SUPPORTED
class HwaLeds : public sys::Builder::HWA::IO::LEDs
{
    public:
    HwaLeds() = default;

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
} hwaLeDs;
#else
class HwaLedsStub : public sys::Builder::HWA::IO::LEDs
{
    public:
    HwaLedsStub() = default;

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
} hwaLEDs;
#endif

#ifdef PROJECT_TARGET_SUPPORT_ADC
class HwaAnalog : public sys::Builder::HWA::IO::Analog
{
    public:
    HwaAnalog() = default;

    bool value(size_t index, uint16_t& value) override
    {
        return board::io::analog::value(index, value);
    }

    uint8_t adcBits() override
    {
        return CORE_MCU_ADC_MAX_VALUE == 1023 ? 10 : 12;
    }
} hwaAnalog;

#else
class HwaAnalogStub : public sys::Builder::HWA::IO::Analog
{
    public:
    HwaAnalogStub() = default;

    bool value(size_t index, uint16_t& value) override
    {
        return false;
    }

    uint8_t adcBits() override
    {
        return 10;
    }
} hwaAnalog;
#endif

#if defined(BUTTONS_SUPPORTED) || defined(ENCODERS_SUPPORTED)
// buttons and encoders have the same data source which is digital input
// this helper class pulls the latest data from board and then feeds it into HwaButtons and HwaEncoders
class HwaDigitalIn
{
    public:
    HwaDigitalIn() = default;

#ifdef BUTTONS_SUPPORTED
    bool buttonState(size_t index, uint8_t& numberOfReadings, uint16_t& states)
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
    bool encoderState(size_t index, uint8_t& numberOfReadings, uint16_t& states)
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
} hwaDigitalIn;
#endif

#ifdef BUTTONS_SUPPORTED

class HwaButtons : public sys::Builder::HWA::IO::Buttons
{
    public:
    HwaButtons() = default;

    bool state(size_t index, uint8_t& numberOfReadings, uint16_t& states) override
    {
        return hwaDigitalIn.buttonState(index, numberOfReadings, states);
    }

    size_t buttonToEncoderIndex(size_t index) override
    {
        return board::io::digitalIn::encoderFromInput(index);
    }
} hwaButtons;
#else
class HwaButtonsStub : public sys::Builder::HWA::IO::Buttons
{
    public:
    HwaButtonsStub() = default;

    bool state(size_t index, uint8_t& numberOfReadings, uint16_t& states) override
    {
        return false;
    }

    size_t buttonToEncoderIndex(size_t index) override
    {
        return 0;
    }
} hwaButtons;
#endif

#ifdef ENCODERS_SUPPORTED

class HwaEncoders : public sys::Builder::HWA::IO::Encoders
{
    public:
    HwaEncoders() = default;

    bool state(size_t index, uint8_t& numberOfReadings, uint16_t& states) override
    {
        return hwaDigitalIn.encoderState(index, numberOfReadings, states);
    }
} hwaEncoders;
#else
class HwaEncodersStub : public sys::Builder::HWA::IO::Encoders
{
    public:
    HwaEncodersStub() = default;

    bool state(size_t index, uint8_t& numberOfReadings, uint16_t& states) override
    {
        return false;
    }
} hwaEncoders;
#endif

class HwaMidiUsb : public sys::Builder::HWA::Protocol::MIDI::USB
{
    public:
    HwaMidiUsb() = default;

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
} hwaMidiUsb;

#ifdef PROJECT_TARGET_SUPPORT_DIN_MIDI
class HwaMidiDin : public sys::Builder::HWA::Protocol::MIDI::DIN
{
    public:
    HwaMidiDin() = default;

    bool supported() override
    {
        return true;
    }

    bool init() override
    {
        static constexpr uint32_t BAUDRATE = 31250;
        return board::uart::init(PROJECT_TARGET_UART_CHANNEL_DIN, BAUDRATE) == board::initStatus_t::OK;
    }

    bool deInit() override
    {
        board::uart::deInit(PROJECT_TARGET_UART_CHANNEL_DIN);
        return true;
    }

    bool setLoopback(bool state) override
    {
        board::uart::setLoopbackState(PROJECT_TARGET_UART_CHANNEL_DIN, state);
        return true;
    }

    bool read(uint8_t& value) override
    {
        if (board::uart::read(PROJECT_TARGET_UART_CHANNEL_DIN, value))
        {
            board::io::indicators::indicateTraffic(board::io::indicators::source_t::UART,
                                                   board::io::indicators::direction_t::INCOMING);

            return true;
        }

        return false;
    }

    bool write(uint8_t& value) override
    {
        if (board::uart::write(PROJECT_TARGET_UART_CHANNEL_DIN, value))
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
            return board::uart::isInitialized(PROJECT_TARGET_UART_CHANNEL_DIN);
        }

        return false;
    }
} hwaMidiDin;
#else
class HwaMidiDinStub : public sys::Builder::HWA::Protocol::MIDI::DIN
{
    public:
    HwaMidiDinStub() = default;

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
} hwaMidiDin;
#endif

#ifdef PROJECT_TARGET_SUPPORT_BLE
class HwaMidiBle : public sys::Builder::HWA::Protocol::MIDI::BLE
{
    public:
    HwaMidiBle() = default;

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
        return core::mcu::timing::ms();
    }
} hwaMidiBle;
#else
class HwaMidiBleStub : public sys::Builder::HWA::Protocol::MIDI::BLE
{
    public:
    HwaMidiBleStub() = default;

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
} hwaMidiBle;
#endif

#ifdef PROJECT_TARGET_SUPPORT_TOUCHSCREEN
class HwaTouchscreen : public sys::Builder::HWA::IO::Touchscreen
{
    public:
    HwaTouchscreen() = default;

    bool init() override
    {
        static constexpr uint32_t BAUDRATE = 38400;
        return board::uart::init(PROJECT_TARGET_UART_CHANNEL_TOUCHSCREEN, BAUDRATE) == board::initStatus_t::OK;
    }

    bool deInit() override
    {
        return board::uart::deInit(PROJECT_TARGET_UART_CHANNEL_TOUCHSCREEN);
    }

    bool write(uint8_t value) override
    {
        return board::uart::write(PROJECT_TARGET_UART_CHANNEL_TOUCHSCREEN, value);
    }

    bool read(uint8_t& value) override
    {
        return board::uart::read(PROJECT_TARGET_UART_CHANNEL_TOUCHSCREEN, value);
    }

    bool allocated(io::common::Allocatable::interface_t interface) override
    {
#ifdef PROJECT_TARGET_UART_CHANNEL_TOUCHSCREEN
        return board::uart::isInitialized(PROJECT_TARGET_UART_CHANNEL_TOUCHSCREEN);
#else
        return false;
#endif
    }
} hwaTouchscreen;
#else
class HwaTouchscreenStub : public sys::Builder::HWA::IO::Touchscreen
{
    public:
    HwaTouchscreenStub() = default;

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
} hwaTouchscreen;
#endif

#if defined(PROJECT_TARGET_SUPPORT_TOUCHSCREEN) && !defined(PROJECT_TARGET_USB_OVER_SERIAL)
// USB link MCUs don't implement USB-CDC<->UART passthrough
class HwaCdcPassthrough : public sys::Builder::HWA::IO::CDCPassthrough
{
    public:
    HwaCdcPassthrough() = default;

    bool supported() override
    {
        return true;
    }

    bool init() override
    {
        if (CdcLocker::locked())
        {
            return false;
        }

        _passThroughState = true;
        CdcLocker::lock();
        return true;
    }

    bool deInit() override
    {
        _passThroughState = false;
        CdcLocker::unlock();
        return true;
    }

    bool uartRead(uint8_t& value) override
    {
        if (board::uart::read(PROJECT_TARGET_UART_CHANNEL_TOUCHSCREEN, value))
        {
            board::io::indicators::indicateTraffic(board::io::indicators::source_t::UART,
                                                   board::io::indicators::direction_t::INCOMING);

            return true;
        }

        return false;
    }

    bool uartWrite(uint8_t value) override
    {
        if (board::uart::write(PROJECT_TARGET_UART_CHANNEL_TOUCHSCREEN, value))
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
            board::uart::init(PROJECT_TARGET_UART_CHANNEL_TOUCHSCREEN, baudRate, true);
        }
    }

    bool allocated(io::common::Allocatable::interface_t interface) override
    {
        return CdcLocker::locked();
    }

    private:
    bool _passThroughState = false;
} hwaCdcPassthrough;
#else
class HwaCdcPassthroughStub : public sys::Builder::HWA::IO::CDCPassthrough
{
    public:
    HwaCdcPassthroughStub() = default;

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
} hwaCdcPassthrough;
#endif

#ifdef PROJECT_TARGET_SUPPORT_DISPLAY
class HwaDisplay : public sys::Builder::HWA::IO::Display
{
    public:
    HwaDisplay() = default;

    bool init() override
    {
        // for i2c, consider ALREADY_INIT status a success
        return board::i2c::init(PROJECT_TARGET_I2C_CHANNEL_DISPLAY, board::i2c::clockSpeed_t::S400K) != board::initStatus_t::ERROR;
    }

    bool write(uint8_t address, uint8_t* buffer, size_t size) override
    {
        return board::i2c::write(PROJECT_TARGET_I2C_CHANNEL_DISPLAY, address, buffer, size);
    }

    bool deviceAvailable(uint8_t address) override
    {
        return board::i2c::deviceAvailable(PROJECT_TARGET_I2C_CHANNEL_DISPLAY, address);
    }
} hwaDisplay;
#else
class HwaDisplayStub : public sys::Builder::HWA::IO::Display
{
    public:
    HwaDisplayStub() = default;

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
} hwaDisplay;
#endif

class HwaSystem : public sys::Builder::HWA::System
{
    public:
    HwaSystem()
    {
        MIDIDispatcher.listen(messaging::eventType_t::SYSTEM,
                              [](const messaging::event_t& event)
                              {
                                  switch (event.systemMessage)
                                  {
                                  case messaging::systemMessage_t::FACTORY_RESET_START:
                                  {
                                      board::usb::deInit();
                                      board::io::indicators::indicateFactoryReset();
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

        if (core::mcu::timing::ms() - lastCheckTime > USB_CONN_CHECK_TIME)
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
            lastCheckTime       = core::mcu::timing::ms();
        }

        if (!CdcLocker::locked())
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
        auto value = static_cast<uint32_t>(type);

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
} hwaSystem;

class HwaBuilder : public ::sys::Builder::HWA
{
    public:
    HwaBuilder() = default;

    ::sys::Builder::HWA::IO& io() override
    {
        return _hwaIo;
    }

    ::sys::Builder::HWA::Protocol& protocol() override
    {
        return _hwaProtocol;
    }

    ::sys::Builder::HWA::System& system() override
    {
        return hwaSystem;
    }

    ::sys::Builder::HWA::Database& database() override
    {
        return hwaDatabase;
    }

    private:
    class HwaIo : public ::sys::Builder::HWA::IO
    {
        public:
        ::sys::Builder::HWA::IO::LEDs& leds() override
        {
            return hwaLeDs;
        }

        ::sys::Builder::HWA::IO::Analog& analog() override
        {
            return hwaAnalog;
        }

        ::sys::Builder::HWA::IO::Buttons& buttons() override
        {
            return hwaButtons;
        }

        ::sys::Builder::HWA::IO::Encoders& encoders() override
        {
            return hwaEncoders;
        }

        ::sys::Builder::HWA::IO::Touchscreen& touchscreen() override
        {
            return hwaTouchscreen;
        }

        ::sys::Builder::HWA::IO::CDCPassthrough& cdcPassthrough() override
        {
            return hwaCdcPassthrough;
        }

        ::sys::Builder::HWA::IO::Display& display() override
        {
            return hwaDisplay;
        }
    } _hwaIo;

    class HwaProtocol : public ::sys::Builder::HWA::Protocol
    {
        public:
        ::sys::Builder::HWA::Protocol::MIDI& midi() override
        {
            return _hwaMidi;
        }

        private:
        class HWAMIDI : public ::sys::Builder::HWA::Protocol::MIDI
        {
            public:
            ::sys::Builder::HWA::Protocol::MIDI::USB& usb() override
            {
                return hwaMidiUsb;
            }

            ::sys::Builder::HWA::Protocol::MIDI::DIN& din() override
            {
                return hwaMidiDin;
            }

            ::sys::Builder::HWA::Protocol::MIDI::BLE& ble() override
            {
                return hwaMidiBle;
            }
        } _hwaMidi;
    } _hwaProtocol;
} hwa;

#if defined(PROJECT_TARGET_SUPPORT_TOUCHSCREEN) && !defined(PROJECT_TARGET_USB_OVER_SERIAL)
namespace board::usb
{
    void onCDCsetLineEncoding(uint32_t baudRate)
    {
        hwaCdcPassthrough.onCDCsetLineEncoding(baudRate);
    }
}    // namespace board::usb
#endif

#ifdef APP_USE_LOGGER
CORE_LOGGER_CREATE(APP_LOGGER, [](const char* message)
                   {
                       return board::usb::writeCDC((uint8_t*)&message[0], strlen(message));
                   });
#endif

sys::Builder  builderInstance(hwa);
sys::Instance systemInstance(builderInstance.hwa(), builderInstance.components());

int main()
{
    systemInstance.init();

    while (true)
    {
        systemInstance.run();
    }

    return 1;
}