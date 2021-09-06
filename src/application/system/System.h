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

#pragma once

#include <functional>
#include "io/common/CInfo.h"
#include "sysex/src/SysExConf.h"
#include "CustomIDs.h"
#include "database/Database.h"
#include "midi/src/MIDI.h"
#include "io/buttons/Buttons.h"
#include "io/encoders/Encoders.h"
#include "io/analog/Analog.h"
#include "io/leds/LEDs.h"
#include "io/touchscreen/Touchscreen.h"
#include "bootloader/FwSelector/FwSelector.h"

class System
{
    public:
    enum class block_t : uint8_t
    {
        global,
        buttons,
        encoders,
        analog,
        leds,
        display,
        touchscreen,
        AMOUNT
    };

    enum class presetSetting_t : uint8_t
    {
        activePreset,
        presetPreserve,
        AMOUNT
    };

    enum class midiFeature_t : uint8_t
    {
        standardNoteOff,
        runningStatus,
        mergeEnabled,
        dinEnabled,
        passToDIN,
        AMOUNT
    };

    enum class midiMerge_t : uint8_t
    {
        mergeType,
        mergeUSBchannel,
        mergeDINchannel,
        AMOUNT
    };

    enum class midiMergeType_t
    {
        DINtoUSB,
        DINtoDIN,
        AMOUNT
    };

    enum class serialPeripheral_t : uint8_t
    {
        dinMIDI,
        touchscreen
    };

    class Section
    {
        public:
        Section() = default;

        enum class global_t : uint8_t
        {
            midiFeatures,
            midiMerge,
            presets,
            AMOUNT
        };

        enum class button_t : uint8_t
        {
            type,
            midiMessage,
            midiID,
            velocity,
            midiChannel,
            AMOUNT
        };

        enum class encoder_t : uint8_t
        {
            enable,
            invert,
            mode,
            midiID,
            midiChannel,
            pulsesPerStep,
            acceleration,
            midiID_MSB,
            remoteSync,
            AMOUNT
        };

        enum class analog_t : uint8_t
        {
            enable,
            invert,
            type,
            midiID,
            midiID_MSB,
            lowerLimit,
            lowerLimit_MSB,
            upperLimit,
            upperLimit_MSB,
            midiChannel,
            AMOUNT
        };

        enum class leds_t : uint8_t
        {
            testColor,
            testBlink,
            global,
            activationID,
            rgbEnable,
            controlType,
            activationValue,
            midiChannel,
            AMOUNT
        };

        enum class display_t : uint8_t
        {
            features,
            setting,
            AMOUNT
        };

        enum class touchscreen_t : uint8_t
        {
            setting,
            xPos,
            yPos,
            width,
            height,
            onScreen,
            offScreen,
            pageSwitchEnabled,
            pageSwitchIndex,
            analogPage,
            analogStartXCoordinate,
            analogEndXCoordinate,
            analogStartYCoordinate,
            analogEndYCoordinate,
            analogType,
            analogResetOnRelease,
            AMOUNT
        };
    };

    using usbConnectionHandler_t = std::function<void()>;
    using uniqueID_t             = std::array<uint8_t, UID_BITS / 8>;

    class HWA
    {
        public:
        HWA() = default;

        virtual bool init()                                                                      = 0;
        virtual void reboot(FwSelector::fwType_t type)                                           = 0;
        virtual void registerOnUSBconnectionHandler(usbConnectionHandler_t usbConnectionHandler) = 0;
        virtual bool serialPeripheralAllocated(serialPeripheral_t peripheral)                    = 0;
        virtual bool uniqueID(uniqueID_t& uniqueID)                                              = 0;
    };

    System(HWA&             hwa,
           ComponentInfo&   cInfo,
           Database&        database,
           MIDI&            midi,
           IO::Buttons&     buttons,
           IO::Encoders&    encoders,
           IO::Analog&      analog,
           IO::LEDs&        leds,
           IO::Display&     display,
           IO::Touchscreen& touchscreen)
        : _sysExConf(
              _sysExDataHandler,
              _sysExMID)
        , _hwa(hwa)
        , _cInfo(cInfo)
        , _database(database)
        , _midi(midi)
        , _buttons(buttons)
        , _encoders(encoders)
        , _analog(analog)
        , _leds(leds)
        , _display(display)
        , _touchscreen(touchscreen)
        , _sysExDataHandler(*this)
        , _dbHandlers(*this)
        , _touchScreenHandlers(*this)
    {}

    bool            init();
    void            run();
    void            handleSysEx(const uint8_t* array, size_t size);
    bool            sendCInfo(Database::block_t dbBlock, uint16_t componentID);
    bool            isMIDIfeatureEnabled(midiFeature_t feature);
    midiMergeType_t midiMergeType();

    private:
    enum class initAction_t : uint8_t
    {
        asIs,
        init,
        deInit
    };

    class SysExDataHandler : public SysExConf::DataHandler
    {
        public:
        SysExDataHandler(System& system)
            : _system(system)
        {}

        uint8_t get(uint8_t block, uint8_t section, uint16_t index, uint16_t& value) override;
        uint8_t set(uint8_t block, uint8_t section, uint16_t index, uint16_t newValue) override;
        uint8_t customRequest(uint16_t request, CustomResponse& customResponse) override;
        void    sendResponse(uint8_t* array, uint16_t size) override;

        private:
        System& _system;
    };

    class DBhandlers : public Database::Handlers
    {
        public:
        DBhandlers(System& system)
            : _system(system)
        {}

        void presetChange(uint8_t preset) override;
        void factoryResetStart() override;
        void factoryResetDone() override;
        void initialized() override;

        private:
        System& _system;
    };

    class TouchScreenHandlers : public IO::Touchscreen::EventNotifier
    {
        public:
        TouchScreenHandlers(System& system)
            : _system(system)
        {}

        void button(size_t index, bool state) override;
        void analog(size_t index, uint16_t value, uint16_t min, uint16_t max) override;
        void screenChange(size_t screenID) override;

        private:
        System& _system;
    };

    void                             checkComponents();
    void                             checkMIDI();
    void                             configureMIDI();
    bool                             onGet(uint8_t block, uint8_t section, size_t index, uint16_t& value);
    bool                             onSet(uint8_t block, uint8_t section, size_t index, uint16_t newValue);
    bool                             onCustomRequest(size_t value);
    void                             onWrite(uint8_t* sysExArray, size_t size);
    void                             backup();
    void                             forceComponentRefresh();
    Database::block_t                dbBlock(uint8_t index);
    Database::Section::global_t      dbSection(Section::global_t section);
    Database::Section::button_t      dbSection(Section::button_t section);
    Database::Section::encoder_t     dbSection(Section::encoder_t section);
    Database::Section::analog_t      dbSection(Section::analog_t section);
    Database::Section::leds_t        dbSection(Section::leds_t section);
    Database::Section::display_t     dbSection(Section::display_t section);
    Database::Section::touchscreen_t dbSection(Section::touchscreen_t section);
    uint8_t                          onGetGlobal(Section::global_t section, size_t index, uint16_t& value);
    uint8_t                          onGetButtons(Section::button_t section, size_t index, uint16_t& value);
    uint8_t                          onGetEncoders(Section::encoder_t section, size_t index, uint16_t& value);
    uint8_t                          onGetAnalog(Section::analog_t section, size_t index, uint16_t& value);
    uint8_t                          onGetLEDs(Section::leds_t section, size_t index, uint16_t& value);
    uint8_t                          onGetDisplay(Section::display_t section, size_t index, uint16_t& value);
    uint8_t                          onGetTouchscreen(Section::touchscreen_t section, size_t index, uint16_t& value);
    uint8_t                          onSetGlobal(Section::global_t section, size_t index, uint16_t newValue);
    uint8_t                          onSetButtons(Section::button_t section, size_t index, uint16_t newValue);
    uint8_t                          onSetEncoders(Section::encoder_t section, size_t index, uint16_t newValue);
    uint8_t                          onSetAnalog(Section::analog_t section, size_t index, uint16_t newValue);
    uint8_t                          onSetLEDs(Section::leds_t section, size_t index, uint16_t newValue);
    uint8_t                          onSetDisplay(Section::display_t section, size_t index, uint16_t newValue);
    uint8_t                          onSetTouchscreen(Section::touchscreen_t section, size_t index, uint16_t newValue);

    SysExConf           _sysExConf;
    HWA&                _hwa;
    ComponentInfo&      _cInfo;
    Database&           _database;
    MIDI&               _midi;
    IO::Buttons&        _buttons;
    IO::Encoders&       _encoders;
    IO::Analog&         _analog;
    IO::LEDs&           _leds;
    IO::Display&        _display;
    IO::Touchscreen&    _touchscreen;
    SysExDataHandler    _sysExDataHandler;
    DBhandlers          _dbHandlers;
    TouchScreenHandlers _touchScreenHandlers;

    const SysExConf::manufacturerID_t _sysExMID = {
        SYSEX_MANUFACTURER_ID_0,
        SYSEX_MANUFACTURER_ID_1,
        SYSEX_MANUFACTURER_ID_2
    };

    static constexpr uint8_t SERIAL_PERIPHERAL_ALLOCATED_ERROR = 80;

    uint32_t _lastCinfoMsgTime[static_cast<uint8_t>(Database::block_t::AMOUNT)] = {};
    bool     _backupRequested                                                   = false;

    //map sysex sections to sections in db
    const Database::Section::global_t _sysEx2DB_global[static_cast<uint8_t>(Section::global_t::AMOUNT)] = {
        Database::Section::global_t::midiFeatures,
        Database::Section::global_t::midiMerge,
        Database::Section::global_t::AMOUNT,    //unused
    };

    const Database::Section::button_t _sysEx2DB_button[static_cast<uint8_t>(Section::button_t::AMOUNT)] = {
        Database::Section::button_t::type,
        Database::Section::button_t::midiMessage,
        Database::Section::button_t::midiID,
        Database::Section::button_t::velocity,
        Database::Section::button_t::midiChannel
    };

    const Database::Section::encoder_t _sysEx2DB_encoder[static_cast<uint8_t>(Section::encoder_t::AMOUNT)] = {
        Database::Section::encoder_t::enable,
        Database::Section::encoder_t::invert,
        Database::Section::encoder_t::mode,
        Database::Section::encoder_t::midiID,
        Database::Section::encoder_t::midiChannel,
        Database::Section::encoder_t::pulsesPerStep,
        Database::Section::encoder_t::acceleration,
        Database::Section::encoder_t::midiID,
        Database::Section::encoder_t::remoteSync
    };

    const Database::Section::analog_t _sysEx2DB_analog[static_cast<uint8_t>(Section::analog_t::AMOUNT)] = {
        Database::Section::analog_t::enable,
        Database::Section::analog_t::invert,
        Database::Section::analog_t::type,
        Database::Section::analog_t::midiID,
        Database::Section::analog_t::midiID,
        Database::Section::analog_t::lowerLimit,
        Database::Section::analog_t::lowerLimit,
        Database::Section::analog_t::upperLimit,
        Database::Section::analog_t::upperLimit,
        Database::Section::analog_t::midiChannel
    };

    const Database::Section::leds_t _sysEx2DB_leds[static_cast<uint8_t>(Section::leds_t::AMOUNT)] = {
        Database::Section::leds_t::AMOUNT,
        Database::Section::leds_t::AMOUNT,
        Database::Section::leds_t::global,
        Database::Section::leds_t::activationID,
        Database::Section::leds_t::rgbEnable,
        Database::Section::leds_t::controlType,
        Database::Section::leds_t::activationValue,
        Database::Section::leds_t::midiChannel,
    };

    const Database::Section::display_t _sysEx2DB_display[static_cast<uint8_t>(Section::display_t::AMOUNT)] = {
        Database::Section::display_t::features,
        Database::Section::display_t::setting,
    };

    const Database::Section::touchscreen_t _sysEx2DB_touchscreen[static_cast<uint8_t>(Section::touchscreen_t::AMOUNT)] = {
        Database::Section::touchscreen_t::setting,
        Database::Section::touchscreen_t::xPos,
        Database::Section::touchscreen_t::yPos,
        Database::Section::touchscreen_t::width,
        Database::Section::touchscreen_t::height,
        Database::Section::touchscreen_t::onScreen,
        Database::Section::touchscreen_t::offScreen,
        Database::Section::touchscreen_t::pageSwitchEnabled,
        Database::Section::touchscreen_t::pageSwitchIndex,
        Database::Section::touchscreen_t::analogPage,
        Database::Section::touchscreen_t::analogStartXCoordinate,
        Database::Section::touchscreen_t::analogEndXCoordinate,
        Database::Section::touchscreen_t::analogStartYCoordinate,
        Database::Section::touchscreen_t::analogEndYCoordinate,
        Database::Section::touchscreen_t::analogType,
        Database::Section::touchscreen_t::analogResetOnRelease,
    };
};