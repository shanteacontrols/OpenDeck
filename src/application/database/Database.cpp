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
#include "core/src/general/Helpers.h"
#include "util/conversion/Conversion.h"
#include "util/configurable/Configurable.h"

/// Helper macro for easier entry and exit from system block.
/// Important: ::init must called before trying to use this macro.
#define SYSTEM_BLOCK_ENTER(code)                                                                                                   \
    {                                                                                                                              \
        LESSDB::setLayout(_layout.layout(Layout::type_t::system));                                                                 \
        code                                                                                                                       \
            LESSDB::setLayout(_layout.layout(Layout::type_t::user), _userDataStartAddress + (_lastPresetAddress * _activePreset)); \
    }

Database::Instance::Instance(LESSDB::StorageAccess& storageAccess,
                             Layout&                layout,
                             bool                   initializeData)
    : LESSDB(storageAccess)
    , _layout(layout)
    , _initializeData(initializeData)
{
    ConfigHandler.registerConfig(
        System::Config::block_t::global,
        // read
        [this](uint8_t section, size_t index, uint16_t& value) {
            return sysConfigGet(static_cast<System::Config::Section::global_t>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value) {
            return sysConfigSet(static_cast<System::Config::Section::global_t>(section), index, value);
        });
}

bool Database::Instance::init(Handlers& handlers)
{
    registerHandlers(handlers);
    return init();
}

bool Database::Instance::init()
{
    if (!LESSDB::init())
    {
        return false;
    }

    uint32_t systemBlockUsage = 0;

    // set only system block for now
    if (!LESSDB::setLayout(_layout.layout(Layout::type_t::system)))
    {
        return false;
    }

    systemBlockUsage      = LESSDB::currentDBsize();
    _userDataStartAddress = LESSDB::nextParameterAddress();

    // now set the user layout
    if (!LESSDB::setLayout(_layout.layout(Layout::type_t::user), _userDataStartAddress))
    {
        return false;
    }

    _lastPresetAddress = LESSDB::nextParameterAddress();

    // get theoretical maximum of presets
    _supportedPresets = (LESSDB::dbSize() - systemBlockUsage) / (LESSDB::currentDBsize() + systemBlockUsage);

    // limit by hardcoded limit
    _supportedPresets = CONSTRAIN(_supportedPresets, 0, Config::MAX_PRESETS);

    _uid = layoutUID(_layout.layout(Layout::type_t::user), _supportedPresets);

    bool returnValue = true;

    if (!isSignatureValid())
    {
        returnValue = factoryReset();
    }
    else
    {
        SYSTEM_BLOCK_ENTER(
            _activePreset = read(0,
                                 static_cast<uint8_t>(Config::Section::system_t::presets),
                                 static_cast<size_t>(Config::presetSetting_t::activePreset));)

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

    if (returnValue)
    {
        _initialized = true;

        if (_handlers != nullptr)
        {
            _handlers->initialized();
        }
    }

    return returnValue;
}

bool Database::Instance::isInitialized()
{
    return _initialized;
}

/// Performs full factory reset of data in database.
bool Database::Instance::factoryReset()
{
    if (_handlers != nullptr)
    {
        _handlers->factoryResetStart();
    }

    if (!clear())
    {
        return false;
    }

    if (_initializeData)
    {
        auto layoutDescriptor = _layout.layout(Layout::type_t::system);

        // system layout first
        if (!LESSDB::setLayout(_layout.layout(Layout::type_t::system), 0))
        {
            return false;
        }

        if (!initData(LESSDB::factoryResetType_t::full))
        {
            return false;
        }

        for (int i = _supportedPresets - 1; i >= 0; i--)
        {
            if (!setPresetInternal(i))
            {
                return false;
            }

            if (!initData(LESSDB::factoryResetType_t::full))
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
bool Database::Instance::setPreset(uint8_t preset)
{
    if (preset >= _supportedPresets)
    {
        return false;
    }

    _activePreset = preset;

    bool returnValue;

    SYSTEM_BLOCK_ENTER(
        returnValue = update(0,
                             static_cast<uint8_t>(Config::Section::system_t::presets),
                             static_cast<size_t>(Config::presetSetting_t::activePreset),
                             preset);)

    if (returnValue)
    {
        if (_handlers != nullptr)
        {
            _handlers->presetChange(preset);
        }
    }

    return returnValue;
}

/// Used to set new database layout (preset) without writing to database.
/// For internal use only.
/// param [in]: preset  New preset to set.
/// returns: False if specified preset isn't supported, true otherwise.
bool Database::Instance::setPresetInternal(uint8_t preset)
{
    if (preset >= _supportedPresets)
    {
        return false;
    }

    _activePreset = preset;
    LESSDB::setLayout(_layout.layout(Layout::type_t::user), _userDataStartAddress + (_lastPresetAddress * _activePreset));

    return true;
}

/// Retrieves currently active preset.
uint8_t Database::Instance::getPreset()
{
    return _activePreset;
}

/// Retrieves number of presets possible to store in database.
/// Preset is simply another database layout copy.
uint8_t Database::Instance::getSupportedPresets()
{
    return _supportedPresets;
}

/// Enables or disables preservation of preset setting.
/// If preservation is enabled, configured preset will be loaded on board power on.
/// Otherwise, first preset will be loaded instead.
bool Database::Instance::setPresetPreserveState(bool state)
{
    bool returnValue;

    SYSTEM_BLOCK_ENTER(
        returnValue = update(0,
                             static_cast<uint8_t>(Config::Section::system_t::presets),
                             static_cast<size_t>(Config::presetSetting_t::presetPreserve),
                             state);)

    return returnValue;
}

/// Checks if preset preservation setting is enabled or disabled.
/// returns: True if preset preservation is enabled, false otherwise.
bool Database::Instance::getPresetPreserveState()
{
    bool returnValue;

    SYSTEM_BLOCK_ENTER(
        returnValue = read(0,
                           static_cast<uint8_t>(Config::Section::system_t::presets),
                           static_cast<size_t>(Config::presetSetting_t::presetPreserve));)

    return returnValue;
}

/// Checks if database has been already initialized by checking DB_BLOCK_ID.
/// returns: True if valid, false otherwise.
bool Database::Instance::isSignatureValid()
{
    uint16_t signature;

    SYSTEM_BLOCK_ENTER(
        signature = read(0,
                         static_cast<uint8_t>(Config::Section::system_t::uid),
                         0);)

    return _uid == signature;
}

/// Updates unique database UID.
/// UID is written to first two database locations.
bool Database::Instance::setUID()
{
    bool returnValue;

    SYSTEM_BLOCK_ENTER(
        returnValue = update(0, static_cast<uint8_t>(Config::Section::system_t::uid), 0, _uid);)

    return returnValue;
}

void Database::Instance::registerHandlers(Handlers& handlers)
{
    _handlers = &handlers;
}

std::optional<uint8_t> Database::Instance::sysConfigGet(System::Config::Section::global_t section, size_t index, uint16_t& value)
{
    int32_t readValue = 0;
    uint8_t result    = System::Config::status_t::errorRead;

    switch (section)
    {
    case System::Config::Section::global_t::presets:
    {
        auto setting = static_cast<Config::presetSetting_t>(index);

        switch (setting)
        {
        case Config::presetSetting_t::activePreset:
        {
            readValue = getPreset();
            result    = System::Config::status_t::ack;
        }
        break;

        case Config::presetSetting_t::presetPreserve:
        {
            readValue = getPresetPreserveState();
            result    = System::Config::status_t::ack;
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

std::optional<uint8_t> Database::Instance::sysConfigSet(System::Config::Section::global_t section, size_t index, uint16_t value)
{
    uint8_t result    = System::Config::status_t::errorWrite;
    bool    writeToDb = true;

    switch (section)
    {
    case System::Config::Section::global_t::presets:
    {
        auto setting = static_cast<Config::presetSetting_t>(index);

        switch (setting)
        {
        case Config::presetSetting_t::activePreset:
        {
            if (value < getSupportedPresets())
            {
                setPreset(value);
                result    = System::Config::status_t::ack;
                writeToDb = false;
            }
            else
            {
                result = System::Config::status_t::errorNotSupported;
            }
        }
        break;

        case Config::presetSetting_t::presetPreserve:
        {
            if ((value <= 1) && (value >= 0))
            {
                setPresetPreserveState(value);
                result    = System::Config::status_t::ack;
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

    if ((result == System::Config::status_t::ack) && writeToDb)
    {
        result = update(Util::Conversion::sys2DBsection(section), index, value) ? System::Config::status_t::ack : System::Config::status_t::errorWrite;
    }

    return result;
}

__attribute__((weak)) void Database::Instance::customInitGlobal()
{
}

__attribute__((weak)) void Database::Instance::customInitButtons()
{
}

__attribute__((weak)) void Database::Instance::customInitEncoders()
{
}

__attribute__((weak)) void Database::Instance::customInitAnalog()
{
}

__attribute__((weak)) void Database::Instance::customInitLEDs()
{
}

__attribute__((weak)) void Database::Instance::customInitDisplay()
{
}

__attribute__((weak)) void Database::Instance::customInitTouchscreen()
{
}