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

#include "dbms/src/LESSDB.h"

///
/// \addtogroup eeprom
/// @{
///

class Database : public LESSDB
{
    public:
    Database(bool (&readCallback)(uint32_t address, sectionParameterType_t type, int32_t& value), bool (&writeCallback)(uint32_t address, int32_t value, sectionParameterType_t type), size_t maxSize)
        : LESSDB(readCallback, writeCallback, maxSize)
    {}

    enum class block_t : uint8_t
    {
        global,
        buttons,
        encoders,
        analog,
        leds,
        display,
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

        enum class display_t : uint8_t
        {
            features,
            setting,
            AMOUNT
        };
    };

    using LESSDB::read;
    using LESSDB::update;

    template<typename T>
    int32_t read(T section, size_t index)
    {
        block_t blockIndex = block(section);
        return LESSDB::read(static_cast<uint8_t>(blockIndex), static_cast<uint8_t>(section), index);
    }

    template<typename T>
    bool read(T section, size_t index, int32_t& value)
    {
        block_t blockIndex = block(section);
        return LESSDB::read(static_cast<uint8_t>(blockIndex), static_cast<uint8_t>(section), index, value);
    }

    template<typename T>
    bool update(T section, size_t index, int32_t value)
    {
        block_t blockIndex = block(section);
        return LESSDB::update(static_cast<uint8_t>(blockIndex), static_cast<uint8_t>(section), index, value);
    }

    bool    init();
    bool    factoryReset(LESSDB::factoryResetType_t type);
    uint8_t getSupportedPresets();
    bool    isSignatureValid();
    bool    setPreset(uint8_t preset);
    uint8_t getPreset();
    void    setPresetPreserveState(bool state);
    bool    getPresetPreserveState();
    void    setPresetChangeHandler(void (*presetChangeHandler)(uint8_t preset));

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

    block_t block(Section::display_t section)
    {
        return block_t::display;
    }

    void     writeCustomValues();
    uint16_t getDbUID();
    void     setDbUID(uint16_t uid);

    ///
    /// \brief User-specified callback called when preset is changed.
    ///
    void (*presetChangeHandler)(uint8_t preset) = nullptr;

    ///
    /// \brief Holds total memory usage for single preset (without system block).
    ///
    uint32_t presetMemoryUsage;

    ///
    /// \brief Total size of system block.
    /// Used to set correct offset in database for user layout.
    ///
    uint32_t systemBlockUsage;

    ///
    /// \brief Holds total number of supported presets.
    ///
    uint16_t supportedPresets;

    ///
    /// \brief Holds currently active preset.
    ///
    uint8_t activePreset = 0;
};