/*

Copyright 2015-2020 Igor Petrovic

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

#include "OpenDeck.h"
#include "board/Board.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Interrupt.h"
#include "core/src/general/Reset.h"
#include "io/common/CInfo.h"

class DBhandlers : public Database::Handlers
{
    public:
    DBhandlers() {}

    void presetChange(uint8_t preset) override
    {
        if (presetChangeHandler != nullptr)
            presetChangeHandler(preset);
    }

    void factoryResetStart() override
    {
        if (factoryResetStartHandler != nullptr)
            factoryResetStartHandler();
    }

    void factoryResetDone() override
    {
        if (factoryResetDoneHandler != nullptr)
            factoryResetDoneHandler();
    }

    void initialized() override
    {
        if (initHandler != nullptr)
            initHandler();
    }

    //actions which these handlers should take depend on objects making
    //up the entire system to be initialized
    //therefore in interface we are calling these function pointers which
    // are set in application once we have all objects ready
    void (*presetChangeHandler)(uint8_t preset) = nullptr;
    void (*factoryResetStartHandler)()          = nullptr;
    void (*factoryResetDoneHandler)()           = nullptr;
    void (*initHandler)()                       = nullptr;
} dbHandlers;

class StorageAccess : public LESSDB::StorageAccess
{
    public:
    StorageAccess() {}

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
Database database(dbHandlers, storageAccess,
#ifdef __AVR__
                  true
#else
                  false
#endif
);

#ifdef LEDS_SUPPORTED
class HWALEDs : public IO::LEDs::HWA
{
    public:
    HWALEDs() {}

    void setState(size_t index, bool state) override
    {
        if (stateHandler != nullptr)
            stateHandler(index, state);
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
        return Board::UART::init(UART_CHANNEL_TOUCHSCREEN, 115200);
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
    HWAEncoders() {}

    uint8_t state(size_t index) override
    {
        return Board::io::getEncoderPairState(index);
    }
} hwaEncoders;

class HWAButtons : public IO::Buttons::HWA
{
    public:
    HWAButtons() {}

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

#ifdef ANALOG_SUPPORTED
class HWAAnalog : public IO::Analog::HWA
{
    public:
    HWAAnalog() {}

    bool value(size_t index, uint16_t& value) override
    {
        return Board::io::analogValue(index, value);
    }
} hwaAnalog;

#include "io/analog/Filter.h"

IO::AnalogFilterSMA<3> analogFilter;
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
    HWAU8X8Stub() {}

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

#ifdef ADC_12_BIT
#define ADC_RESOLUTION IO::Analog::adcType_t::adc12bit
#else
#define ADC_RESOLUTION IO::Analog::adcType_t::adc10bit
#endif

ComponentInfo   cinfo;
MIDI            midi;
IO::Common      digitalInputCommon;
IO::U8X8        u8x8(hwaU8X8);
IO::Display     display(u8x8, database);
IO::Touchscreen touchscreen(database);
IO::LEDs        leds(hwaLEDs, database);
IO::Analog      analog(hwaAnalog, ADC_RESOLUTION, analogFilter, database, midi, leds, display, cinfo);
IO::Buttons     buttons(hwaButtons, buttonsFilter, database, midi, leds, display, cinfo);
IO::Encoders    encoders(hwaEncoders, database, midi, display, cinfo);
SysConfig       sysConfig(database, midi, buttons, encoders, analog, leds, display, touchscreen);

void OpenDeck::init()
{
    Board::init();

    database.init();
    sysConfig.init();
    leds.init();
    encoders.init();
    display.init(true);

#ifdef TOUCHSCREEN_SUPPORTED
    touchscreen.registerModel(IO::Touchscreen::Model::model_t::nextion, &touchscreenModelNextion);
    touchscreen.registerModel(IO::Touchscreen::Model::model_t::viewtech, &touchscreenModelViewtech);
#endif

    touchscreen.init();

    dbHandlers.factoryResetStartHandler = []() {
        leds.setAllOff();
    };

    dbHandlers.presetChangeHandler = [](uint8_t preset) {
        leds.midiToState(MIDI::messageType_t::programChange, preset, 0, 0, true);

        if (display.init(false))
            display.displayMIDIevent(IO::Display::eventType_t::in, IO::Display::event_t::presetChange, preset, 0, 0);
    };

    cinfo.registerHandler([](Database::block_t dbBlock, SysExConf::sysExParameter_t componentID) {
        return sysConfig.sendCInfo(dbBlock, componentID);
    });

    analog.setButtonHandler([](uint8_t analogIndex, bool value) {
        buttons.processButton(analogIndex + MAX_NUMBER_OF_BUTTONS, value);
    });

    touchscreen.setButtonHandler([](size_t index, bool state) {
        buttons.processButton(MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + index, state);
        cinfo.send(Database::block_t::touchscreen, index);
    });

    touchscreen.setScreenChangeHandler([](size_t screenID) {
        leds.refresh();
    });

    // on startup, indicate current program for all channels (if any leds have program change assigned as control mode)
    for (int i = 0; i < 16; i++)
        leds.midiToState(MIDI::messageType_t::programChange, 0, 0, i, false);

    //don't configure this handler before initializing database to avoid mcu reset if
    //factory reset is needed initially
    dbHandlers.factoryResetDoneHandler = []() {
        core::reset::mcuReset();
    };

    hwaLEDs.registerHandler([](size_t index, bool state) {
#if MAX_NUMBER_OF_LEDS > 0
#if MAX_NUMBER_OF_TOUCHSCREEN_BUTTONS != 0
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
}

void OpenDeck::checkComponents()
{
    if (sysConfig.isProcessingEnabled())
    {
        if (Board::io::isInputDataAvailable())
        {
            buttons.update();
            encoders.update();
        }

        analog.update();

        leds.checkBlinking();
        display.update();

        touchscreen.update();
    }
}

void OpenDeck::checkMIDI()
{
    auto processMessage = [](MIDI::interface_t interface) {
        //new message
        auto    messageType = midi.getType(interface);
        uint8_t data1       = midi.getData1(interface);
        uint8_t data2       = midi.getData2(interface);
        uint8_t channel     = midi.getChannel(interface);

        switch (messageType)
        {
        case MIDI::messageType_t::systemExclusive:
            //process sysex messages only from usb interface
            if (interface == MIDI::interface_t::usb)
                sysConfig.handleSysEx(midi.getSysExArray(interface), midi.getSysExArrayLength(interface));
            break;

        case MIDI::messageType_t::noteOn:
        case MIDI::messageType_t::noteOff:
        case MIDI::messageType_t::controlChange:
        case MIDI::messageType_t::programChange:
            if (messageType == MIDI::messageType_t::programChange)
                digitalInputCommon.setProgram(channel, data1);

            if (messageType == MIDI::messageType_t::noteOff)
                data2 = 0;

            leds.midiToState(messageType, data1, data2, channel, false);

            switch (messageType)
            {
            case MIDI::messageType_t::noteOn:
                display.displayMIDIevent(IO::Display::eventType_t::in, IO::Display::event_t::noteOn, data1, data2, channel + 1);
                break;

            case MIDI::messageType_t::noteOff:
                display.displayMIDIevent(IO::Display::eventType_t::in, IO::Display::event_t::noteOff, data1, data2, channel + 1);
                break;

            case MIDI::messageType_t::controlChange:
                display.displayMIDIevent(IO::Display::eventType_t::in, IO::Display::event_t::controlChange, data1, data2, channel + 1);
                break;

            case MIDI::messageType_t::programChange:
                display.displayMIDIevent(IO::Display::eventType_t::in, IO::Display::event_t::programChange, data1, data2, channel + 1);
                break;

            default:
                break;
            }

            if (messageType == MIDI::messageType_t::programChange)
                database.setPreset(data1);

            if (messageType == MIDI::messageType_t::controlChange)
            {
                for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
                {
                    if (!database.read(Database::Section::encoder_t::remoteSync, i))
                        continue;

                    if (database.read(Database::Section::encoder_t::mode, i) != static_cast<int32_t>(IO::Encoders::type_t::tControlChange))
                        continue;

                    if (database.read(Database::Section::encoder_t::midiChannel, i) != channel)
                        continue;

                    if (database.read(Database::Section::encoder_t::midiID, i) != data1)
                        continue;

                    encoders.setValue(i, data2);
                }
            }
            break;

        case MIDI::messageType_t::sysRealTimeClock:
            leds.checkBlinking(true);
            break;

        case MIDI::messageType_t::sysRealTimeStart:
            leds.resetBlinking();
            leds.checkBlinking(true);
            break;

        default:
            break;
        }
    };

    //note: mega/uno
    //"fake" usb interface - din data is stored as usb data so use usb callback to read the usb
    //packet stored in midi object
#ifdef DIN_MIDI_SUPPORTED
    if (
        sysConfig.isMIDIfeatureEnabled(SysConfig::midiFeature_t::dinEnabled) &&
        sysConfig.isMIDIfeatureEnabled(SysConfig::midiFeature_t::passToDIN))
    {
        //pass the message to din
        if (midi.read(MIDI::interface_t::usb, MIDI::filterMode_t::fullDIN))
            processMessage(MIDI::interface_t::usb);
    }
    else
    {
        if (midi.read(MIDI::interface_t::usb))
            processMessage(MIDI::interface_t::usb);
    }
#else
    if (midi.read(MIDI::interface_t::usb))
        processMessage(MIDI::interface_t::usb);
#endif

#ifdef DIN_MIDI_SUPPORTED
    if (sysConfig.isMIDIfeatureEnabled(SysConfig::midiFeature_t::dinEnabled))
    {
        if (sysConfig.isMIDIfeatureEnabled(SysConfig::midiFeature_t::mergeEnabled))
        {
            auto mergeType = sysConfig.midiMergeType();

            switch (mergeType)
            {
            case SysConfig::midiMergeType_t::DINtoUSB:
                //dump everything from DIN MIDI in to USB MIDI out
                midi.read(MIDI::interface_t::din, MIDI::filterMode_t::fullUSB);
                break;

                // case SysConfig::midiMergeType_t::DINtoDIN:
                //loopback is automatically configured here
                // break;

                // case SysConfig::midiMergeType_t::odMaster:
                //already configured
                // break;

            case SysConfig::midiMergeType_t::odSlaveInitial:
                //handle the traffic regulary until slave is properly configured
                //(upon receiving message from master)
                if (midi.read(MIDI::interface_t::din))
                    processMessage(MIDI::interface_t::din);
                break;

            default:
                break;
            }
        }
        else
        {
            if (midi.read(MIDI::interface_t::din))
                processMessage(MIDI::interface_t::din);
        }
    }
#endif
}

void OpenDeck::update()
{
    checkMIDI();
    checkComponents();
}