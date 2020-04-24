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

    void init() override
    {
        Board::eeprom::init();
    }

    uint32_t size() override
    {
        return Board::eeprom::size();
    }

    void clear() override
    {
        Board::eeprom::clear(0, Board::eeprom::size());
    }

    bool read(uint32_t address, int32_t& value, LESSDB::sectionParameterType_t type) override
    {
        return Board::eeprom::read(address, value, type);
    }

    bool write(uint32_t address, int32_t value, LESSDB::sectionParameterType_t type) override
    {
        return Board::eeprom::write(address, value, type);
    }

    size_t paramUsage(LESSDB::sectionParameterType_t type) override
    {
        return Board::eeprom::paramUsage(type);
    }
} storageAccess;

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

    void (*stateHandler)(size_t index, bool state) = nullptr;
} hwaLEDs;

#ifdef TOUCHSCREEN_SUPPORTED
#ifdef OD_BOARD_BERGAMOT
//bergamot uses SDW touchscreen display

#include "io/touchscreen/model/sdw/SDW.h"

class SDWHWA : public IO::Touchscreen::Model::HWA
{
    public:
    SDWHWA() {}

    bool init() override
    {
        Board::UART::init(UART_TOUCHSCREEN_CHANNEL, 38400);
        return true;
    }

    bool write(uint8_t* buffer, size_t size)
    {
        for (size_t i = 0; i < size; i++)
        {
            if (!Board::UART::write(UART_TOUCHSCREEN_CHANNEL, buffer[i]))
                return false;
        }

        return true;
    }

    bool read(uint8_t* buffer, size_t& size) override
    {
        uint8_t data;

        if (!Board::UART::read(UART_TOUCHSCREEN_CHANNEL, data))
            return false;

        size      = 1;
        buffer[0] = data;
        return true;
    }
} sdwHWA;

SDW touchscreenModel(sdwHWA);
#else
//nextion by default
#include "io/touchscreen/model/nextion/Nextion.h"

class NextionHWA : public IO::Touchscreen::Model::HWA
{
    public:
    NextionHWA() {}

    bool init() override
    {
        Board::UART::init(UART_TOUCHSCREEN_CHANNEL, 38400);
        return true;
    }

    bool write(uint8_t* buffer, size_t size)
    {
        for (size_t i = 0; i < size; i++)
        {
            if (!Board::UART::write(UART_TOUCHSCREEN_CHANNEL, buffer[i]))
                return false;
        }

        return true;
    }

    bool read(uint8_t* buffer, size_t& size) override
    {
        uint8_t data;

        if (!Board::UART::read(UART_TOUCHSCREEN_CHANNEL, data))
            return false;

        size      = 1;
        buffer[0] = data;
        return true;
    }
} nextionHWA;

Nextion touchscreenModel(nextionHWA);
#endif
#endif

// clang-format off
ComponentInfo                       cinfo;
Database                            database(dbHandlers, storageAccess);
MIDI                                midi;
IO::Common                          digitalInputCommon;
#ifdef DISPLAY_SUPPORTED
IO::Display                         display(database);
#endif
#ifdef TOUCHSCREEN_SUPPORTED
IO::Touchscreen                     touchscreen(touchscreenModel);
#endif
IO::LEDs                            leds(hwaLEDs, database);
#ifdef DISPLAY_SUPPORTED
#ifdef ADC_10_BIT
IO::Analog                          analog(IO::Analog::adcType_t::adc10bit, database, midi, leds, display, cinfo);
#else
IO::Analog                          analog(IO::Analog::adcType_t::adc12bit, database, midi, leds, display, cinfo);
#endif
#else
#ifdef ADC_10_BIT
IO::Analog                          analog(IO::Analog::adcType_t::adc10bit, database, midi, leds, cinfo);
#else
IO::Analog                          analog(IO::Analog::adcType_t::adc12bit, database, midi, leds, cinfo);
#endif
#endif
#ifdef DISPLAY_SUPPORTED
IO::Buttons                         buttons(database, midi, leds, display, cinfo);
#else
IO::Buttons                         buttons(database, midi, leds, cinfo);
#endif
#ifdef DISPLAY_SUPPORTED
IO::Encoders                        encoders(database, midi, display, cinfo);
#else
IO::Encoders                        encoders(database, midi, cinfo);
#endif
#ifdef DISPLAY_SUPPORTED
SysConfig                           sysConfig(database, midi, buttons, encoders, analog, leds, display);
#else
SysConfig                           sysConfig(database, midi, buttons, encoders, analog, leds);
#endif
//clang-format on

void OpenDeck::init()
{
    Board::init();

    dbHandlers.presetChangeHandler = [](uint8_t preset) {
        leds.midiToState(MIDI::messageType_t::programChange, preset, 0, 0, true);

#ifdef DISPLAY_SUPPORTED
        if (display.init(false))
            display.displayMIDIevent(IO::Display::eventType_t::in, IO::Display::event_t::presetChange, preset, 0, 0);
#endif
    };

    dbHandlers.factoryResetStartHandler = []() {
        leds.setAllOff();
        core::timing::waitMs(1000);
    };

    database.init();
    sysConfig.init();
    leds.init();
    encoders.init();
#ifdef DISPLAY_SUPPORTED
    display.init(true);
#endif

#ifdef TOUCHSCREEN_SUPPORTED
#ifdef OD_BOARD_BERGAMOT
    touchscreen.init();
    touchscreen.setScreen(1);
#endif
#endif

    cinfo.registerHandler([](Database::block_t dbBlock, SysExConf::sysExParameter_t componentID) {
        return sysConfig.sendCInfo(dbBlock, componentID);
    });

    analog.setButtonHandler([](uint8_t analogIndex, bool value) {
        buttons.processButton(analogIndex + MAX_NUMBER_OF_BUTTONS, value);
    });

#ifdef TOUCHSCREEN_SUPPORTED
    touchscreen.setButtonHandler([](uint8_t index, bool state) {
        buttons.processButton(MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + index, state);
    });
#endif

    // on startup, indicate current program for all channels (if any leds have program change assigned as control mode)
    for (int i = 0; i < 16; i++)
        leds.midiToState(MIDI::messageType_t::programChange, 0, 0, i, false);

    //don't configure this handler before initializing database to avoid mcu reset if
    //factory reset is needed initially
    dbHandlers.factoryResetDoneHandler = []() {
        core::reset::mcuReset();
    };

    hwaLEDs.stateHandler = [](size_t index, bool state) {
#if MAX_NUMBER_OF_LEDS > 0
#if MAX_TOUCHSCREEN_BUTTONS != 0
        if (index >= MAX_NUMBER_OF_LEDS)
            touchscreen.setButtonState(MAX_NUMBER_OF_LEDS - index, state);
        else
            Board::io::writeLEDstate(index, state);
#else
        Board::io::writeLEDstate(index, state);
#endif
#else
#ifdef TOUCHSCREEN_SUPPORTED
        touchscreen.setButtonState(index, state);
#endif
#endif
    };

    Board::io::ledFlashStartup(Board::checkNewRevision());
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
#ifdef DISPLAY_SUPPORTED
        display.update();
#endif

#ifdef TOUCHSCREEN_SUPPORTED
        touchscreen.update();
#endif
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

#ifdef DISPLAY_SUPPORTED
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
#endif

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
    if (midi.read(MIDI::interface_t::usb))
        processMessage(MIDI::interface_t::usb);

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