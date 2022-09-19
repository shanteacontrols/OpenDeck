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

#include <inttypes.h>
#include "core/src/util/Util.h"
#include "util/conversion/Conversion.h"
#include "util/configurable/Configurable.h"

/// Helper macro for easier entry and exit from system block.
/// Important: ::init must called before trying to use this macro.
#define SYSTEM_BLOCK_ENTER(code)                                                                                                   \
    {                                                                                                                              \
        LESSDB::setLayout(_layout.layout(Layout::type_t::SYSTEM));                                                                 \
        code                                                                                                                       \
            LESSDB::setLayout(_layout.layout(Layout::type_t::USER), _userDataStartAddress + (_lastPresetAddress * _activePreset)); \
    }

Database::Admin::Admin(LESSDB::StorageAccess& storageAccess,
                       Layout&                layout,
                       bool                   initializeData)
    : LESSDB(storageAccess)
    , _layout(layout)
    , INITIALIZE_DATA(initializeData)
{
    ConfigHandler.registerConfig(
        System::Config::block_t::GLOBAL,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sysConfigGet(static_cast<System::Config::Section::global_t>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sysConfigSet(static_cast<System::Config::Section::global_t>(section), index, value);
        });
}

bool Database::Admin::init(Handlers& handlers)
{
    registerHandlers(handlers);
    return init();
}

bool Database::Admin::init()
{
    if (!LESSDB::init())
    {
        return false;
    }

    uint32_t systemBlockUsage = 0;

    // set only system block for now
    if (!LESSDB::setLayout(_layout.layout(Layout::type_t::SYSTEM)))
    {
        return false;
    }

    systemBlockUsage      = LESSDB::currentDBsize();
    _userDataStartAddress = LESSDB::nextParameterAddress();

    // now set the user layout
    if (!LESSDB::setLayout(_layout.layout(Layout::type_t::USER), _userDataStartAddress))
    {
        return false;
    }

    _lastPresetAddress = LESSDB::nextParameterAddress();

    // get theoretical maximum of presets
    _supportedPresets = (LESSDB::dbSize() - systemBlockUsage) / (LESSDB::currentDBsize() + systemBlockUsage);

    // limit by hardcoded limit
    _supportedPresets = core::util::CONSTRAIN(_supportedPresets,
                                              static_cast<size_t>(0),
                                              Config::MAX_PRESETS);

    _uid = layoutUID(_layout.layout(Layout::type_t::USER), _supportedPresets);

    bool retVal = true;

    if (!isSignatureValid())
    {
        retVal = factoryReset();
    }
    else
    {
        SYSTEM_BLOCK_ENTER(
            _activePreset = read(0,
                                 static_cast<uint8_t>(Config::Section::system_t::PRESETS),
                                 static_cast<size_t>(Config::presetSetting_t::ACTIVE_PRESET));)

        if (getPresetPreserveState())
        {
            // don't write anything to database in this case - setup preset only internally
            setPresetInternal(_activePreset);
        }
        else
        {
            if (_activePreset != 0)
            {
                // preset preservation is not set which means preset must be 0
                // in this case it is not so overwrite it with 0
                setPreset(0);
            }
            else
            {
                setPresetInternal(0);
            }
        }
    }

    if (retVal)
    {
        _initialized = true;

        if (_handlers != nullptr)
        {
            _handlers->initialized();
        }
    }

    return retVal;
}

bool Database::Admin::isInitialized()
{
    return _initialized;
}

/// Performs full factory reset of data in database.
bool Database::Admin::factoryReset()
{
    if (_handlers != nullptr)
    {
        _handlers->factoryResetStart();
    }

    if (!clear())
    {
        return false;
    }

    if (INITIALIZE_DATA)
    {
        auto layoutDescriptor = _layout.layout(Layout::type_t::SYSTEM);

        // system layout first
        if (!LESSDB::setLayout(_layout.layout(Layout::type_t::SYSTEM), 0))
        {
            return false;
        }

        if (!initData(LESSDB::factoryResetType_t::FULL))
        {
            return false;
        }

        for (int i = _supportedPresets - 1; i >= 0; i--)
        {
            if (!setPresetInternal(i))
            {
                return false;
            }

            if (!initData(LESSDB::factoryResetType_t::FULL))
            {
                return false;
            }

            // perform custom init as well
            customInitGlobal();
            customInitButtons();
            customInitEncoders();
            customInitAnalog();
            customInitLEDs();
            customInitDisplay();
            customInitTouchscreen();
        }

        if (!setPresetPreserveState(false))
        {
            return false;
        }

        if (!setUID())
        {
            return false;
        }
    }
    else
    {
        if (!setPresetInternal(0))
        {
            return false;
        }
    }

    if (_handlers != nullptr)
    {
        _handlers->factoryResetDone();
    }

    return true;
}

/// Used to set new database layout (preset).
/// param [in]: preset  New preset to set.
/// returns: False if specified preset isn't supported, true otherwise.
bool Database::Admin::setPreset(uint8_t preset)
{
    if (preset >= _supportedPresets)
    {
        return false;
    }

    _activePreset = preset;

    bool retVal;

    SYSTEM_BLOCK_ENTER(
        retVal = update(0,
                        static_cast<uint8_t>(Config::Section::system_t::PRESETS),
                        static_cast<size_t>(Config::presetSetting_t::ACTIVE_PRESET),
                        preset);)

    if (retVal)
    {
        if (_handlers != nullptr)
        {
            _handlers->presetChange(preset);
        }
    }

    return retVal;
}

/// Used to set new database layout (preset) without writing to database.
/// For internal use only.
/// param [in]: preset  New preset to set.
/// returns: False if specified preset isn't supported, true otherwise.
bool Database::Admin::setPresetInternal(uint8_t preset)
{
    if (preset >= _supportedPresets)
    {
        return false;
    }

    _activePreset = preset;
    LESSDB::setLayout(_layout.layout(Layout::type_t::USER), _userDataStartAddress + (_lastPresetAddress * _activePreset));

    return true;
}

/// Retrieves currently active preset.
uint8_t Database::Admin::getPreset()
{
    return _activePreset;
}

/// Retrieves number of presets possible to store in database.
/// Preset is simply another database layout copy.
uint8_t Database::Admin::getSupportedPresets()
{
    return _supportedPresets;
}

/// Enables or disables preservation of preset setting.
/// If preservation is enabled, configured preset will be loaded on board power on.
/// Otherwise, first preset will be loaded instead.
bool Database::Admin::setPresetPreserveState(bool state)
{
    bool retVal;

    SYSTEM_BLOCK_ENTER(
        retVal = update(0,
                        static_cast<uint8_t>(Config::Section::system_t::PRESETS),
                        static_cast<size_t>(Config::presetSetting_t::PRESET_PRESERVE),
                        state);)

    return retVal;
}

/// Checks if preset preservation setting is enabled or disabled.
/// returns: True if preset preservation is enabled, false otherwise.
bool Database::Admin::getPresetPreserveState()
{
    bool retVal;

    SYSTEM_BLOCK_ENTER(
        retVal = read(0,
                      static_cast<uint8_t>(Config::Section::system_t::PRESETS),
                      static_cast<size_t>(Config::presetSetting_t::PRESET_PRESERVE));)

    return retVal;
}

/// Checks if database has been already initialized by checking DB_BLOCK_ID.
/// returns: True if valid, false otherwise.
bool Database::Admin::isSignatureValid()
{
    uint16_t signature;

    SYSTEM_BLOCK_ENTER(
        signature = read(0,
                         static_cast<uint8_t>(Config::Section::system_t::UID),
                         0);)

    return _uid == signature;
}

/// Updates unique database UID.
/// UID is written to first two database locations.
bool Database::Admin::setUID()
{
    bool retVal;

    SYSTEM_BLOCK_ENTER(
        retVal = update(0, static_cast<uint8_t>(Config::Section::system_t::UID), 0, _uid);)

    return retVal;
}

void Database::Admin::registerHandlers(Handlers& handlers)
{
    _handlers = &handlers;
}

std::optional<uint8_t> Database::Admin::sysConfigGet(System::Config::Section::global_t section, size_t index, uint16_t& value)
{
    uint32_t readValue = 0;
    uint8_t  result    = System::Config::status_t::ERROR_READ;

    switch (section)
    {
    case System::Config::Section::global_t::PRESETS:
    {
        auto setting = static_cast<Config::presetSetting_t>(index);

        switch (setting)
        {
        case Config::presetSetting_t::ACTIVE_PRESET:
        {
            readValue = getPreset();
            result    = System::Config::status_t::ACK;
        }
        break;

        case Config::presetSetting_t::PRESET_PRESERVE:
        {
            readValue = getPresetPreserveState();
            result    = System::Config::status_t::ACK;
        }
        break;

        default:
            break;
        }
    }
    break;

    default:
        return std::nullopt;
    }

    value = readValue;
    return result;
}

std::optional<uint8_t> Database::Admin::sysConfigSet(System::Config::Section::global_t section, size_t index, uint16_t value)
{
    uint8_t result    = System::Config::status_t::ERROR_WRITE;
    bool    writeToDb = true;

    switch (section)
    {
    case System::Config::Section::global_t::PRESETS:
    {
        auto setting = static_cast<Config::presetSetting_t>(index);

        switch (setting)
        {
        case Config::presetSetting_t::ACTIVE_PRESET:
        {
            if (value < getSupportedPresets())
            {
                setPreset(value);
                result    = System::Config::status_t::ACK;
                writeToDb = false;
            }
            else
            {
                result = System::Config::status_t::ERROR_NOT_SUPPORTED;
            }
        }
        break;

        case Config::presetSetting_t::PRESET_PRESERVE:
        {
            if ((value <= 1) && (value >= 0))
            {
                setPresetPreserveState(value);
                result    = System::Config::status_t::ACK;
                writeToDb = false;
            }
        }
        break;

        default:
            break;
        }
        break;
    }
    break;

    default:
        return std::nullopt;
    }

    if ((result == System::Config::status_t::ACK) && writeToDb)
    {
        result = update(Util::Conversion::sys2DBsection(section), index, value) ? System::Config::status_t::ACK : System::Config::status_t::ERROR_WRITE;
    }

    return result;
}

__attribute__((weak)) void Database::Admin::customInitGlobal()
{
}

__attribute__((weak)) void Database::Admin::customInitButtons()
{
}

__attribute__((weak)) void Database::Admin::customInitEncoders()
{
}

__attribute__((weak)) void Database::Admin::customInitAnalog()
{
}

__attribute__((weak)) void Database::Admin::customInitLEDs()
{
}

__attribute__((weak)) void Database::Admin::customInitDisplay()
{
}

__attribute__((weak)) void Database::Admin::customInitTouchscreen()
{
}