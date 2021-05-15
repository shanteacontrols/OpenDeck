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
#include "core/src/general/Helpers.h"
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
        if (_stateHandler != nullptr)
            _stateHandler(index, static_cast<bool>(brightness));
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
            boardRGBindex = Board::io::rgbIndex_t::r;
            break;

        case IO::LEDs::rgbIndex_t::g:
            boardRGBindex = Board::io::rgbIndex_t::g;
            break;

        case IO::LEDs::rgbIndex_t::b:
            boardRGBindex = Board::io::rgbIndex_t::b;
            break;

        default:
            return 0;
        }

        return Board::io::rgbSignalIndex(rgbIndex, boardRGBindex);
#else
        return 0;
#endif
    }

    void registerHandler(void (*fptr)(size_t index, bool state))
    {
        _stateHandler = fptr;
    }

    private:
    void (*_stateHandler)(size_t index, bool state) = nullptr;
} hwaLEDs;
#else
class HWALEDsStub : public IO::LEDs::HWA
{
    public:
    HWALEDsStub() {}

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

#if defined(BUTTONS_SUPPORTED) || defined(ENCODERS_SUPPORTED)
//buttons and encoders have the same data source which is digital input
//this helper class pulls the latest data from board and then feeds it into HWAButtons and HWAEncoders
class HWADigitalIn
{
    public:
    HWADigitalIn() = default;

#ifdef BUTTONS_SUPPORTED
    bool buttonState(size_t index, uint8_t& numberOfReadings, uint32_t& states)
    {
        //if encoder under this index is enabled, just return false state each time
        //side note: don't bother with references to dependencies here, just use global database object
        if (database.read(Database::Section::encoder_t::enable, Board::io::encoderIndex(index)))
            return false;

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

        //construct encoder pair readings
        //encoder signal is made of A and B signals
        //take each bit of A signal and B signal and append it to states variable in order
        //latest encoder readings should be in LSB bits

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
} hwaDigitalIn;
#endif

#ifdef BUTTONS_SUPPORTED

#include "io/buttons/Filter.h"

IO::ButtonsFilter buttonsFilter;

class HWAButtons : public IO::Buttons::HWA
{
    public:
    HWAButtons() = default;

    bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
    {
        return hwaDigitalIn.buttonState(index, numberOfReadings, states);
    }
} hwaButtons;
#else
class HWAButtonsStub : public IO::Buttons::HWA
{
    public:
    HWAButtonsStub() {}

    bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
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

#ifdef ENCODERS_SUPPORTED

#include "io/encoders/Filter.h"

IO::EncodersFilter encodersFilter;

class HWAEncoders : public IO::Encoders::HWA
{
    public:
    HWAEncoders() = default;

    bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
    {
        return hwaDigitalIn.encoderState(index, numberOfReadings, states);
    }
} hwaEncoders;
#else
class HWAEncodersStub : public IO::Encoders::HWA
{
    public:
    HWAEncodersStub() {}

    bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
    {
        return false;
    }
} hwaEncoders;

class EncodersFilterStub : public IO::Encoders::Filter
{
    public:
    EncodersFilterStub() {}

    virtual bool isFiltered(size_t index,
                            IO::Encoders::position_t position,
                            IO::Encoders::position_t& filteredPosition,
                            uint32_t sampleTakenTime) override
    {
        return false;
    }

    virtual void reset(size_t index)
    {
    }

    virtual uint32_t lastMovementTime(size_t index)
    {
        return 0;
    }
} encodersFilter;
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
IO::AnalogFilter analogFilter(ADC_RESOLUTION, 2);
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

    void reboot(FwSelector::fwType_t type) override
    {
        auto value = static_cast<uint8_t>(type);

        Board::bootloader::setMagicBootValue(value);
        Board::reboot();
    }

    void enableDINMIDI(bool loopback) override
    {
#ifdef DIN_MIDI_SUPPORTED
        if (_dinMIDIenabled && (loopback == _dinMIDIloopbackEnabled))
            return;    //nothing do do

        if (!_dinMIDIenabled)
            Board::UART::init(UART_CHANNEL_DIN, UART_BAUDRATE_MIDI_STD);

        Board::UART::setLoopbackState(UART_CHANNEL_DIN, loopback);

        _dinMIDIenabled         = true;
        _dinMIDIloopbackEnabled = loopback;
#endif
    }

    void disableDINMIDI() override
    {
#ifdef DIN_MIDI_SUPPORTED
        if (!_dinMIDIenabled)
            return;    //nothing to do

        Board::UART::deInit(UART_CHANNEL_DIN);
        _dinMIDIenabled         = false;
        _dinMIDIloopbackEnabled = false;
#endif
    }

    void registerOnUSBconnectionHandler(System::usbConnectionHandler_t usbConnectionHandler)
    {
        _usbConnectionHandler = std::move(usbConnectionHandler);
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
                    if (_usbConnectionHandler != nullptr)
                        _usbConnectionHandler();
                }
            }

            lastConnectionState = newState;
            lastCheckTime       = core::timing::currentRunTimeMs();
        }
    }

    private:
    /// Time in milliseconds after which USB connection state should be checked
    static constexpr uint32_t USB_CONN_CHECK_TIME = 2000;

#ifdef DIN_MIDI_SUPPORTED
    bool _dinMIDIenabled         = false;
    bool _dinMIDIloopbackEnabled = false;
#endif

    System::usbConnectionHandler_t _usbConnectionHandler = nullptr;
} hwaSystem;

MIDI            midi(hwaMIDI);
ComponentInfo   cInfo;
IO::U8X8        u8x8(hwaU8X8);
IO::Display     display(u8x8, database);
IO::LEDs        leds(hwaLEDs, database);
IO::Analog      analog(hwaAnalog, analogFilter, database, midi, leds, display, cInfo);
IO::Buttons     buttons(hwaButtons, buttonsFilter, 1, database, midi, leds, display, cInfo);
IO::Encoders    encoders(hwaEncoders, encodersFilter, 1, database, midi, display, cInfo);
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
        hwaSystem.checkUSBconnection();
        sys.run();
    }

    return 1;
}