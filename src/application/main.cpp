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

#ifndef USB_LINK_MCU
#include "database/Database.h"
#include "io/buttons/Buttons.h"
#include "io/analog/Analog.h"
#include "io/encoders/Encoders.h"
#include "io/leds/LEDs.h"
#include "io/display/Display.h"
#include "io/touchscreen/Touchscreen.h"
#include "system/System.h"
#endif
#include "midi/src/MIDI.h"
#include "board/Board.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Interrupt.h"
#include "core/src/general/Reset.h"
#include "io/common/CInfo.h"
#include "board/common/USBMIDIOverSerial/USBMIDIOverSerial.h"
#include "bootloader/FwSelector/FwSelector.h"

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
        switch (type)
        {
        case LESSDB::sectionParameterType_t::word:
            return Board::NVM::read(address, value, Board::NVM::parameterType_t::word);

        case LESSDB::sectionParameterType_t::dword:
            return Board::NVM::read(address, value, Board::NVM::parameterType_t::dword);

        default:
            return Board::NVM::read(address, value, Board::NVM::parameterType_t::byte);
        }
    }

    bool write(uint32_t address, int32_t value, LESSDB::sectionParameterType_t type) override
    {
        switch (type)
        {
        case LESSDB::sectionParameterType_t::word:
            return Board::NVM::write(address, value, Board::NVM::parameterType_t::word);

        case LESSDB::sectionParameterType_t::dword:
            return Board::NVM::write(address, value, Board::NVM::parameterType_t::dword);

        default:
            return Board::NVM::write(address, value, Board::NVM::parameterType_t::byte);
        }
    }

    size_t paramUsage(LESSDB::sectionParameterType_t type) override
    {
        switch (type)
        {
        case LESSDB::sectionParameterType_t::word:
            return Board::NVM::paramUsage(Board::NVM::parameterType_t::word);

        case LESSDB::sectionParameterType_t::dword:
            return Board::NVM::paramUsage(Board::NVM::parameterType_t::dword);

        default:
            return Board::NVM::paramUsage(Board::NVM::parameterType_t::byte);
        }
    }
} storageAccess;
Database database(storageAccess,
#ifdef __AVR__
                  true
#else
                  false
#endif
);

class HWAMIDI : public MIDI::HWA
{
    public:
    HWAMIDI() = default;

    bool init() override
    {
        return true;
    }

    bool dinRead(uint8_t& data) override
    {
#ifdef DIN_MIDI_SUPPORTED
        return Board::UART::read(UART_CHANNEL_DIN, data);
#else
        return false;
#endif
    }

    bool dinWrite(uint8_t data) override
    {
#ifdef DIN_MIDI_SUPPORTED
        return Board::UART::write(UART_CHANNEL_DIN, data);
#else
        return false;
#endif
    }

    bool usbRead(MIDI::USBMIDIpacket_t& USBMIDIpacket) override
    {
        return Board::USB::readMIDI(USBMIDIpacket);
    }

    bool usbWrite(MIDI::USBMIDIpacket_t& USBMIDIpacket) override
    {
        return Board::USB::writeMIDI(USBMIDIpacket);
    }
} hwaMIDI;

#ifdef LEDS_SUPPORTED
class HWALEDs : public IO::LEDs::HWA
{
    public:
    HWALEDs() = default;

    void setState(size_t index, IO::LEDs::brightness_t brightness) override
    {
        if (stateHandler != nullptr)
            stateHandler(index, static_cast<bool>(brightness));
    }

    size_t rgbSingleComponentIndex(size_t rgbIndex, IO::LEDs::rgbIndex_t rgbComponent) override
    {
#if MAX_NUMBER_OF_LEDS > 0
        Board::io::rgbIndex_t boardRGBindex;

        switch (rgbComponent)
        {
        case IO::LEDs::rgbIndex_t::r:
            boardRGBindex = Board::io::rgbIndex_t::r;
            break;

        case IO::LEDs::rgbIndex_t::g:
            boardRGBindex = Board::io::rgbIndex_t::r;
            break;
        case IO::LEDs::rgbIndex_t::b:
            boardRGBindex = Board::io::rgbIndex_t::b;
            break;

        default:
            return 0;
        }

        return Board::io::getRGBaddress(rgbIndex, boardRGBindex);
#else
        return 0;
#endif
    }

    size_t rgbIndex(size_t singleLEDindex) override
    {
#if MAX_NUMBER_OF_LEDS > 0
        return Board::io::getRGBID(singleLEDindex);
#else
        return 0;
#endif
    }

    void setFadeSpeed(size_t transitionSpeed) override
    {
#if MAX_NUMBER_OF_LEDS > 0
        Board::io::setLEDfadeSpeed(transitionSpeed);
#endif
    }

    void registerHandler(void (*fptr)(size_t index, bool state))
    {
        stateHandler = fptr;
    }

    private:
    void (*stateHandler)(size_t index, bool state) = nullptr;
} hwaLEDs;
#else
class HWALEDsStub : public IO::LEDs::HWA
{
    public:
    HWALEDsStub() {}

    void setState(size_t index, bool state) override
    {
    }

    size_t rgbSingleComponentIndex(size_t rgbIndex, IO::LEDs::rgbIndex_t rgbComponent) override
    {
        return 0;
    }

    size_t rgbIndex(size_t singleLEDindex) override
    {
        return 0;
    }

    void setFadeSpeed(size_t transitionSpeed) override
    {
    }

    void registerHandler(void (*fptr)(size_t index, bool state))
    {
    }
} hwaLEDs;
#endif

#ifdef TOUCHSCREEN_SUPPORTED
#include "io/touchscreen/model/nextion/Nextion.h"
#include "io/touchscreen/model/viewtech/Viewtech.h"

//use the same hwa instance for all models
class HWAtouchscreen : public IO::Touchscreen::Model::HWA
{
    public:
    HWAtouchscreen() {}

    bool init() override
    {
        if (Board::UART::init(UART_CHANNEL_TOUCHSCREEN, 115200))
        {
            //add slight delay before display becomes ready on power on
            core::timing::waitMs(1000);

            return true;
        }

        return false;
    }

    bool deInit() override
    {
        return Board::UART::deInit(UART_CHANNEL_TOUCHSCREEN);
    }

    bool write(uint8_t data) override
    {
        return Board::UART::write(UART_CHANNEL_TOUCHSCREEN, data);
    }

    bool read(uint8_t& data) override
    {
        return Board::UART::read(UART_CHANNEL_TOUCHSCREEN, data);
    }
} touchscreenHWA;

Nextion  touchscreenModelNextion(touchscreenHWA);
Viewtech touchscreenModelViewtech(touchscreenHWA);
#endif

#ifdef BUTTONS_SUPPORTED
class HWAEncoders : public IO::Encoders::HWA
{
    public:
    HWAEncoders() = default;

    uint8_t state(size_t index) override
    {
        return Board::io::getEncoderPairState(index);
    }
} hwaEncoders;

class HWAButtons : public IO::Buttons::HWA
{
    public:
    HWAButtons() = default;

    bool state(size_t index) override
    {
        //if encoder under this index is enabled, just return false state each time
        //side note: don't bother with references to dependencies here, just use global database object
        if (database.read(Database::Section::encoder_t::enable, Board::io::getEncoderPair(index)))
            return false;

        return Board::io::getButtonState(index);
    }
} hwaButtons;

#include "io/buttons/Filter.h"

IO::ButtonsFilter buttonsFilter;
#else
class HWAEncodersStub : public IO::Encoders::HWA
{
    public:
    HWAEncodersStub() {}

    uint8_t state(size_t index) override
    {
        return 0;
    }
} hwaEncoders;

class HWAButtonsStub : public IO::Buttons::HWA
{
    public:
    HWAButtonsStub() {}

    bool state(size_t index) override
    {
        return false;
    }
} hwaButtons;

class ButtonsFilterStub : public IO::Buttons::Filter
{
    public:
    ButtonsFilterStub() {}

    bool isFiltered(size_t index, bool state, bool& filteredState) override
    {
        return false;
    }

    void reset(size_t index) override
    {
    }
} buttonsFilter;
#endif

#ifdef ADC_12_BIT
#define ADC_RESOLUTION IO::Analog::adcType_t::adc12bit
#else
#define ADC_RESOLUTION IO::Analog::adcType_t::adc10bit
#endif

#ifdef ANALOG_SUPPORTED
class HWAAnalog : public IO::Analog::HWA
{
    public:
    HWAAnalog() = default;

    bool value(size_t index, uint16_t& value) override
    {
        return Board::io::analogValue(index, value);
    }
} hwaAnalog;

#include "io/analog/Filter.h"

#ifdef __AVR__
IO::AnalogFilter analogFilter(ADC_RESOLUTION, 1);
#else
//stm32 has more sensitive ADC, use more repetitions
IO::AnalogFilter analogFilter(ADC_RESOLUTION, 3);
#endif
#else
class HWAAnalogStub : public IO::Analog::HWA
{
    public:
    HWAAnalogStub() {}

    bool value(size_t index, uint16_t& value) override
    {
        return false;
    }
} hwaAnalog;

class AnalogFilterStub : public IO::Analog::Filter
{
    public:
    AnalogFilterStub() {}

    bool isFiltered(size_t index, uint16_t value, uint16_t& filteredValue) override
    {
        return false;
    }

    void reset(size_t index) override
    {
    }
} analogFilter;
#endif

#ifdef DISPLAY_SUPPORTED
class HWAU8X8 : public IO::U8X8::HWAI2C
{
    public:
    HWAU8X8() {}

    bool init() override
    {
        return Board::I2C::init(I2C_CHANNEL_DISPLAY, Board::I2C::clockSpeed_t::_1kHz);
    }

    bool deInit() override
    {
        return Board::I2C::deInit(I2C_CHANNEL_DISPLAY);
    }

    bool write(uint8_t address, uint8_t* data, size_t size) override
    {
        return Board::I2C::write(I2C_CHANNEL_DISPLAY, address, data, size);
    }
} hwaU8X8;
#else
class HWAU8X8Stub : public IO::U8X8::HWAI2C
{
    public:
    HWAU8X8Stub() = default;

    bool init() override
    {
        return false;
    }

    bool deInit() override
    {
        return false;
    }

    bool write(uint8_t address, uint8_t* data, size_t size) override
    {
        return false;
    }
} hwaU8X8;
#endif

class SystemHWA : public System::HWA
{
    public:
    SystemHWA() = default;

    bool init() override
    {
        Board::init();
        return true;
    }

    bool isDigitalInputAvailable() override
    {
        return Board::io::isInputDataAvailable();
    }

    void reboot(FwSelector::fwType_t type) override
    {
        auto value = static_cast<uint8_t>(type);

        Board::bootloader::setMagicBootValue(value);
        Board::reboot();
    }

    void enableDINMIDI(bool loopback) override
    {
#ifdef DIN_MIDI_SUPPORTED
        if (dinMIDIenabled && (loopback == dinMIDIloopbackEnabled))
            return;    //nothing do do

        if (!dinMIDIenabled)
            Board::UART::init(UART_CHANNEL_DIN, UART_BAUDRATE_MIDI_STD);

        Board::UART::setLoopbackState(UART_CHANNEL_DIN, loopback);

        dinMIDIenabled         = true;
        dinMIDIloopbackEnabled = loopback;
#endif
    }

    void disableDINMIDI() override
    {
#ifdef DIN_MIDI_SUPPORTED
        if (!dinMIDIenabled)
            return;    //nothing to do

        Board::UART::deInit(UART_CHANNEL_DIN);
        dinMIDIenabled         = false;
        dinMIDIloopbackEnabled = false;
#endif
    }

    void registerOnUSBconnectionHandler(System::usbConnectionHandler_t usbConnectionHandler)
    {
        this->usbConnectionHandler = std::move(usbConnectionHandler);
    }

    void checkUSBconnection()
    {
        static uint32_t lastCheckTime       = 0;
        static bool     lastConnectionState = false;

        if (core::timing::currentRunTimeMs() - lastCheckTime > USB_CONN_CHECK_TIME)
        {
            bool newState = Board::USB::isUSBconnected();

            if (newState)
            {
                if (!lastConnectionState)
                {
                    if (usbConnectionHandler != nullptr)
                        usbConnectionHandler();
                }
            }

            lastConnectionState = newState;
        }
    }

    private:
#ifdef DIN_MIDI_SUPPORTED
    bool dinMIDIenabled         = false;
    bool dinMIDIloopbackEnabled = false;
#endif

    /// Time in milliseconds after which USB connection state should be checked
    static constexpr uint32_t USB_CONN_CHECK_TIME = 2000;

    System::usbConnectionHandler_t usbConnectionHandler = nullptr;
} hwaSystem;

MIDI            midi(hwaMIDI);
ComponentInfo   cInfo;
IO::U8X8        u8x8(hwaU8X8);
IO::Display     display(u8x8, database);
IO::LEDs        leds(hwaLEDs, database);
IO::Analog      analog(hwaAnalog, analogFilter, database, midi, leds, display, cInfo);
IO::Buttons     buttons(hwaButtons, buttonsFilter, database, midi, leds, display, cInfo);
IO::Encoders    encoders(hwaEncoders, database, midi, display, cInfo);
IO::Touchscreen touchscreen(database, cInfo);
System          sys(hwaSystem, cInfo, database, midi, buttons, encoders, analog, leds, display, touchscreen);

int main()
{
#ifdef TOUCHSCREEN_SUPPORTED
    touchscreen.registerModel(IO::Touchscreen::Model::model_t::nextion, &touchscreenModelNextion);
    touchscreen.registerModel(IO::Touchscreen::Model::model_t::viewtech, &touchscreenModelViewtech);
#endif

    hwaLEDs.registerHandler([](size_t index, bool state) {
#if MAX_NUMBER_OF_LEDS > 0
#if MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS != 0
        if (index >= MAX_NUMBER_OF_LEDS)
            touchscreen.setIconState(MAX_NUMBER_OF_LEDS - index, state);
        else
            Board::io::writeLEDstate(index, state);
#else
        Board::io::writeLEDstate(index, state);
#endif
#else
        touchscreen.setIconState(index, state);
#endif
    });

    sys.init();

    while (true)
    {
        sys.run();
        hwaSystem.checkUSBconnection();
    }

    return 1;
}