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

#pragma once

#include "sysex/src/SysExConf.h"
#include "CustomIDs.h"
#include "database/Database.h"
#include "midi/src/MIDI.h"
#include "io/buttons/Buttons.h"
#include "io/encoders/Encoders.h"
#include "io/analog/Analog.h"
#include "io/leds/LEDs.h"
#include "io/touchscreen/Touchscreen.h"

class SysConfig
{
    public:
    SysConfig(Database&        database,
              MIDI&            midi,
              IO::Buttons&     buttons,
              IO::Encoders&    encoders,
              IO::Analog&      analog,
              IO::LEDs&        leds,
              IO::Display&     display,
              IO::Touchscreen& touchscreen)
        : sysExConf(
              sysExDataHandler,
              sysExMID,
              SysExConf::paramSize_t::_14bit,
              SysExConf::nrOfParam_t::_32)
        , database(database)
        , midi(midi)
        , buttons(buttons)
        , encoders(encoders)
        , analog(analog)
        , leds(leds)
        , display(display)
        , touchscreen(touchscreen)
        , sysExDataHandler(*this)
    {}

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

    class Section
    {
        public:
        Section() {}

        enum class global_t : uint8_t
        {
            midiFeature,
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
            AMOUNT
        };
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
        odMaster,
        odSlave,
        odSlaveInitial,
        AMOUNT
    };

    void            init();
    void            handleSysEx(const uint8_t* array, size_t size);
    bool            isProcessingEnabled();
    bool            sendCInfo(Database::block_t dbBlock, SysExConf::sysExParameter_t componentID);
    bool            isMIDIfeatureEnabled(midiFeature_t feature);
    midiMergeType_t midiMergeType();

    private:
    using result_t = SysExConf::DataHandler::result_t;

    class SysExDataHandler : public SysExConf::DataHandler
    {
        public:
        SysExDataHandler(SysConfig& sysConfig)
            : sysConfig(sysConfig)
        {}

        result_t get(uint8_t block, uint8_t section, size_t index, SysExConf::sysExParameter_t& value) override;
        result_t set(uint8_t block, uint8_t section, size_t index, SysExConf::sysExParameter_t newValue) override;
        result_t customRequest(size_t request, CustomResponse& customResponse) override;
        void     sendResponse(uint8_t* array, size_t size) override;

        private:
        SysConfig& sysConfig;
    };

    const SysExConf::manufacturerID_t sysExMID = {
        SYSEX_MANUFACTURER_ID_0,
        SYSEX_MANUFACTURER_ID_1,
        SYSEX_MANUFACTURER_ID_2
    };

    SysExConf sysExConf;

    Database&        database;
    MIDI&            midi;
    IO::Buttons&     buttons;
    IO::Encoders&    encoders;
    IO::Analog&      analog;
    IO::LEDs&        leds;
    IO::Display&     display;
    IO::Touchscreen& touchscreen;

    SysExDataHandler sysExDataHandler;

    ///
    /// Used to prevent updating states of all components (analog, LEDs, encoders, buttons).
    ///
    bool processingEnabled = true;

    void configureMIDI();
    bool onGet(uint8_t block, uint8_t section, size_t index, SysExConf::sysExParameter_t& value);
    bool onSet(uint8_t block, uint8_t section, size_t index, SysExConf::sysExParameter_t newValue);
    bool onCustomRequest(size_t value);
    void onWrite(uint8_t* sysExArray, size_t size);
    void backup();

    ///
    /// \brief Configures UART read/write handlers for MIDI module.
    ///
    void setupMIDIoverUART(uint32_t baudRate, bool initRX, bool initTX);

    ///
    /// \brief Configures USB read/write handlers for MIDI module.
    ///
    void setupMIDIoverUSB();

#ifdef DIN_MIDI_SUPPORTED
    void configureMIDImerge(midiMergeType_t mergeType);
    void sendDaisyChainRequest();
#endif

    uint32_t lastCinfoMsgTime[static_cast<uint8_t>(Database::block_t::AMOUNT)];
    bool     backupRequested = false;

    //map sysex sections to sections in db
    const Database::Section::global_t sysEx2DB_global[static_cast<uint8_t>(Section::global_t::AMOUNT)] = {
        Database::Section::global_t::midiFeatures,
        Database::Section::global_t::midiMerge,
        Database::Section::global_t::AMOUNT,    //unused
    };

    const Database::Section::button_t sysEx2DB_button[static_cast<uint8_t>(Section::button_t::AMOUNT)] = {
        Database::Section::button_t::type,
        Database::Section::button_t::midiMessage,
        Database::Section::button_t::midiID,
        Database::Section::button_t::velocity,
        Database::Section::button_t::midiChannel
    };

    const Database::Section::encoder_t sysEx2DB_encoder[static_cast<uint8_t>(Section::encoder_t::AMOUNT)] = {
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

    const Database::Section::analog_t sysEx2DB_analog[static_cast<uint8_t>(Section::analog_t::AMOUNT)] = {
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

    const Database::Section::leds_t sysEx2DB_leds[static_cast<uint8_t>(Section::leds_t::AMOUNT)] = {
        Database::Section::leds_t::AMOUNT,
        Database::Section::leds_t::AMOUNT,
        Database::Section::leds_t::global,
        Database::Section::leds_t::activationID,
        Database::Section::leds_t::rgbEnable,
        Database::Section::leds_t::controlType,
        Database::Section::leds_t::activationValue,
        Database::Section::leds_t::midiChannel,
    };

    const Database::Section::display_t sysEx2DB_display[static_cast<uint8_t>(Section::display_t::AMOUNT)] = {
        Database::Section::display_t::features,
        Database::Section::display_t::setting,
    };

    const Database::Section::touchscreen_t sysEx2DB_touchscreen[static_cast<uint8_t>(Section::touchscreen_t::AMOUNT)] = {
        Database::Section::touchscreen_t::setting,
        Database::Section::touchscreen_t::xPos,
        Database::Section::touchscreen_t::yPos,
        Database::Section::touchscreen_t::width,
        Database::Section::touchscreen_t::height,
        Database::Section::touchscreen_t::onScreen,
        Database::Section::touchscreen_t::offScreen,
        Database::Section::touchscreen_t::pageSwitchEnabled,
        Database::Section::touchscreen_t::pageSwitchIndex,
    };

    enum class initAction_t : uint8_t
    {
        asIs,
        init,
        deInit
    };

    Database::block_t dbBlock(uint8_t index);

    Database::Section::global_t      dbSection(Section::global_t section);
    Database::Section::button_t      dbSection(Section::button_t section);
    Database::Section::encoder_t     dbSection(Section::encoder_t section);
    Database::Section::analog_t      dbSection(Section::analog_t section);
    Database::Section::leds_t        dbSection(Section::leds_t section);
    Database::Section::display_t     dbSection(Section::display_t section);
    Database::Section::touchscreen_t dbSection(Section::touchscreen_t section);

    result_t onGetGlobal(Section::global_t section, size_t index, SysExConf::sysExParameter_t& value);
    result_t onGetButtons(Section::button_t section, size_t index, SysExConf::sysExParameter_t& value);
    result_t onGetEncoders(Section::encoder_t section, size_t index, SysExConf::sysExParameter_t& value);
    result_t onGetAnalog(Section::analog_t section, size_t index, SysExConf::sysExParameter_t& value);
    result_t onGetLEDs(Section::leds_t section, size_t index, SysExConf::sysExParameter_t& value);
    result_t onGetDisplay(Section::display_t section, size_t index, SysExConf::sysExParameter_t& value);
    result_t onGetTouchscreen(Section::touchscreen_t section, size_t index, SysExConf::sysExParameter_t& value);

    result_t onSetGlobal(Section::global_t section, size_t index, SysExConf::sysExParameter_t newValue);
    result_t onSetButtons(Section::button_t section, size_t index, SysExConf::sysExParameter_t newValue);
    result_t onSetEncoders(Section::encoder_t section, size_t index, SysExConf::sysExParameter_t newValue);
    result_t onSetAnalog(Section::analog_t section, size_t index, SysExConf::sysExParameter_t newValue);
    result_t onSetLEDs(Section::leds_t section, size_t index, SysExConf::sysExParameter_t newValue);
    result_t onSetDisplay(Section::display_t section, size_t index, SysExConf::sysExParameter_t newValue);
    result_t onSetTouchscreen(Section::touchscreen_t section, size_t index, SysExConf::sysExParameter_t newValue);
};