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

#pragma once

#include "dbms/src/LESSDB.h"
#include "system/Config.h"

class Database : public LESSDB
{
    public:
    class Handlers
    {
        public:
        Handlers()                                = default;
        virtual void presetChange(uint8_t preset) = 0;
        virtual void factoryResetStart()          = 0;
        virtual void factoryResetDone()           = 0;
        virtual void initialized()                = 0;
    };

    Database(LESSDB::StorageAccess& storageAccess, bool initializeData);

    using sectionParameterType_t = LESSDB::sectionParameterType_t;

    enum class block_t : uint8_t
    {
        global,
        buttons,
        encoders,
        analog,
        leds,
        i2c,
        touchscreen,
        AMOUNT
    };

    class Section
    {
        public:
        Section() {}

        enum class global_t : uint8_t
        {
            midiFeatures,
            midiMerge,
            dmx,
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
            remoteSync,
            AMOUNT
        };

        enum class analog_t : uint8_t
        {
            enable,
            invert,
            type,
            midiID,
            lowerLimit,
            upperLimit,
            midiChannel,
            AMOUNT
        };

        enum class leds_t : uint8_t
        {
            global,
            activationID,
            rgbEnable,
            controlType,
            activationValue,
            midiChannel,
            AMOUNT
        };

        enum class i2c_t : uint8_t
        {
            display,
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

    enum class presetSetting_t : uint8_t
    {
        activePreset,
        presetPreserve,
        AMOUNT
    };

    using LESSDB::read;
    using LESSDB::update;

    template<typename T, typename I>
    int32_t read(T section, I index)
    {
        block_t blockIndex = block(section);
        return LESSDB::read(static_cast<uint8_t>(blockIndex), static_cast<uint8_t>(section), static_cast<size_t>(index));
    }

    template<typename T, typename I>
    bool read(T section, I index, int32_t& value)
    {
        block_t blockIndex = block(section);
        return LESSDB::read(static_cast<uint8_t>(blockIndex), static_cast<uint8_t>(section), static_cast<size_t>(index), value);
    }

    template<typename T, typename I, typename V>
    bool update(T section, I index, V value)
    {
        block_t blockIndex = block(section);
        return LESSDB::update(static_cast<uint8_t>(blockIndex), static_cast<uint8_t>(section), static_cast<size_t>(index), static_cast<int32_t>(value));
    }

    bool    init();
    bool    init(Handlers& handlers);
    bool    factoryReset();
    uint8_t getSupportedPresets();
    bool    setPreset(uint8_t preset);
    uint8_t getPreset();
    bool    isInitialized();
    void    registerHandlers(Handlers& handlers);
    bool    setPresetPreserveState(bool state);
    bool    getPresetPreserveState();

    private:
    block_t block(Section::global_t section)
    {
        return block_t::global;
    }

    block_t block(Section::button_t section)
    {
        return block_t::buttons;
    }

    block_t block(Section::encoder_t section)
    {
        return block_t::encoders;
    }

    block_t block(Section::analog_t section)
    {
        return block_t::analog;
    }

    block_t block(Section::leds_t section)
    {
        return block_t::leds;
    }

    block_t block(Section::i2c_t section)
    {
        return block_t::i2c;
    }

    block_t block(Section::touchscreen_t section)
    {
        return block_t::touchscreen;
    }

    void                   customInitGlobal();
    void                   customInitButtons();
    void                   customInitEncoders();
    void                   customInitAnalog();
    void                   customInitLEDs();
    void                   customInitDisplay();
    void                   customInitTouchscreen();
    bool                   isSignatureValid();
    uint16_t               getDbUID();
    bool                   setDbUID(uint16_t uid);
    bool                   setPresetInternal(uint8_t preset);
    std::optional<uint8_t> sysConfigGet(System::Config::Section::global_t section, size_t index, uint16_t& value);
    std::optional<uint8_t> sysConfigSet(System::Config::Section::global_t section, size_t index, uint16_t value);

    Handlers* _handlers = nullptr;

    const bool _initializeData;

    /// Address at which user data starts (after system block).
    /// Used to set correct offset in database for user layout.
    uint32_t _userDataStartAddress;

    /// Address at which next preset should start.
    /// Used to calculate the start address of next preset.
    uint32_t _lastPresetAddress;

    /// Holds total number of supported presets.
    uint16_t _supportedPresets;

    /// Holds currently active preset.
    uint8_t _activePreset = 0;

    bool _initialized = false;
};