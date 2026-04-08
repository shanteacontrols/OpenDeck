/*

Copyright Igor Petrovic

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

#include "application/util/conversion/conversion.h"
#include "application/util/configurable/configurable.h"

#include "core/util/util.h"

#include <inttypes.h>

using namespace lib::lessdb;

/// Helper macro for easier entry and exit from system block.
/// Important: ::init must called before trying to use this macro.
#define SYSTEM_BLOCK_ENTER(code)                                                                                                   \
    {                                                                                                                              \
        LessDb::setLayout(_layout.layout(Layout::type_t::SYSTEM));                                                                 \
        code                                                                                                                       \
            LessDb::setLayout(_layout.layout(Layout::type_t::USER), _userDataStartAddress + (_lastPresetAddress * _activePreset)); \
    }

database::Admin::Admin(Hwa&    hwa,
                       Layout& layout)
    : LessDb::LessDb(hwa)
    , _layout(layout)
    , INITIALIZE_DATA(hwa.initializeDatabase())
{
    ConfigHandler.registerConfig(
        sys::Config::block_t::GLOBAL,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sysConfigGet(static_cast<sys::Config::Section::global_t>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sysConfigSet(static_cast<sys::Config::Section::global_t>(section), index, value);
        });
}

bool database::Admin::init(Handlers& handlers)
{
    registerHandlers(handlers);
    return init();
}

bool database::Admin::init()
{
    if (!LessDb::init())
    {
        return false;
    }

    uint32_t systemBlockUsage = 0;

    // set only system block for now
    if (!LessDb::setLayout(_layout.layout(Layout::type_t::SYSTEM)))
    {
        return false;
    }

    systemBlockUsage      = LessDb::currentDatabaseSize();
    _userDataStartAddress = LessDb::nextParameterAddress();

    // now set the user layout
    if (!LessDb::setLayout(_layout.layout(Layout::type_t::USER), _userDataStartAddress))
    {
        return false;
    }

    _lastPresetAddress = LessDb::nextParameterAddress();

    // get theoretical maximum of presets
    _supportedPresets = (LessDb::dbSize() - systemBlockUsage) / (LessDb::currentDatabaseSize() + systemBlockUsage);

    // limit by hardcoded limit
    _supportedPresets = core::util::CONSTRAIN(_supportedPresets,
                                              static_cast<size_t>(0),
                                              Config::MAX_PRESETS);

    _uid = layoutUid(_layout.layout(Layout::type_t::USER), _supportedPresets);

    bool retVal = true;

    if (!isSignatureValid())
    {
        retVal = factoryReset();
    }
    else
    {
        _activePreset = readSystemBlock(static_cast<size_t>(Config::systemSetting_t::ACTIVE_PRESET));

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

bool database::Admin::isInitialized()
{
    return _initialized;
}

/// Performs full factory reset of data in database.
bool database::Admin::factoryReset()
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
        if (!LessDb::setLayout(_layout.layout(Layout::type_t::SYSTEM), 0))
        {
            return false;
        }

        if (!initData(factoryResetType_t::FULL))
        {
            return false;
        }

        for (int i = _supportedPresets - 1; i >= 0; i--)
        {
            if (!setPresetInternal(i))
            {
                return false;
            }

            if (!initData(factoryResetType_t::FULL))
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
bool database::Admin::setPreset(uint8_t preset)
{
    if (preset >= _supportedPresets)
    {
        return false;
    }

    _activePreset = preset;

    auto retVal = updateSystemBlock(static_cast<size_t>(Config::systemSetting_t::ACTIVE_PRESET),
                                    preset);

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
bool database::Admin::setPresetInternal(uint8_t preset)
{
    if (preset >= _supportedPresets)
    {
        return false;
    }

    _activePreset = preset;
    LessDb::setLayout(_layout.layout(Layout::type_t::USER), _userDataStartAddress + (_lastPresetAddress * _activePreset));

    return true;
}

/// Retrieves currently active preset.
uint8_t database::Admin::getPreset()
{
    return _activePreset;
}

/// Retrieves number of presets possible to store in database.
/// Preset is simply another database layout copy.
uint8_t database::Admin::getSupportedPresets()
{
    return _supportedPresets;
}

/// Enables or disables preservation of preset setting.
/// If preservation is enabled, configured preset will be loaded on board power on.
/// Otherwise, first preset will be loaded instead.
bool database::Admin::setPresetPreserveState(bool state)
{
    return updateSystemBlock(static_cast<size_t>(Config::systemSetting_t::PRESET_PRESERVE),
                             state);
}

/// Checks if preset preservation setting is enabled or disabled.
/// returns: True if preset preservation is enabled, false otherwise.
bool database::Admin::getPresetPreserveState()
{
    return readSystemBlock(static_cast<size_t>(Config::systemSetting_t::PRESET_PRESERVE));
}

/// Checks if database has been already initialized by checking DB_BLOCK_ID.
/// returns: True if valid, false otherwise.
bool database::Admin::isSignatureValid()
{
    uint16_t signature = readSystemBlock(static_cast<size_t>(Config::systemSetting_t::UID));

    return _uid == signature;
}

/// Updates unique database UID.
/// UID is written to first two database locations.
bool database::Admin::setUID()
{
    return updateSystemBlock(static_cast<size_t>(Config::systemSetting_t::UID), _uid);
}

void database::Admin::registerHandlers(Handlers& handlers)
{
    _handlers = &handlers;
}

std::optional<uint8_t> database::Admin::sysConfigGet(sys::Config::Section::global_t section, size_t index, uint16_t& value)
{
    if (section != sys::Config::Section::global_t::SYSTEM_SETTINGS)
    {
        return std::nullopt;
    }

    if (index >= static_cast<uint8_t>(Config::systemSetting_t::CUSTOM_SYSTEM_SETTING_START))
    {
        return std::nullopt;
    }

    uint32_t readValue = 0;
    uint8_t  result    = sys::Config::Status::ERROR_READ;

    auto setting = static_cast<Config::systemSetting_t>(index);

    switch (setting)
    {
    case Config::systemSetting_t::ACTIVE_PRESET:
    {
        readValue = getPreset();
        result    = sys::Config::Status::ACK;
    }
    break;

    case Config::systemSetting_t::PRESET_PRESERVE:
    {
        readValue = getPresetPreserveState();
        result    = sys::Config::Status::ACK;
    }
    break;

    default:
        break;
    }

    value = readValue;
    return result;
}

std::optional<uint8_t> database::Admin::sysConfigSet(sys::Config::Section::global_t section, size_t index, uint16_t value)
{
    if (section != sys::Config::Section::global_t::SYSTEM_SETTINGS)
    {
        return std::nullopt;
    }

    if (index >= static_cast<uint8_t>(Config::systemSetting_t::CUSTOM_SYSTEM_SETTING_START))
    {
        return std::nullopt;
    }

    uint8_t result  = sys::Config::Status::ERROR_WRITE;
    auto    setting = static_cast<Config::systemSetting_t>(index);

    switch (setting)
    {
    case Config::systemSetting_t::ACTIVE_PRESET:
    {
        if (value < getSupportedPresets())
        {
            setPreset(value);
            result = sys::Config::Status::ACK;
        }
        else
        {
            result = sys::Config::Status::ERROR_NOT_SUPPORTED;
        }
    }
    break;

    case Config::systemSetting_t::PRESET_PRESERVE:
    {
        if ((value <= 1) && (value >= 0))
        {
            setPresetPreserveState(value);
            result = sys::Config::Status::ACK;
        }
    }
    break;

    default:
        return std::nullopt;
    }

    return result;
}

uint16_t database::Admin::readSystemBlock(size_t index)
{
    uint16_t value = 0;

    SYSTEM_BLOCK_ENTER(
        value = LessDb::read(0,
                             static_cast<uint8_t>(Config::Section::system_t::SYSTEM_SETTINGS),
                             index);)

    return value;
}

bool database::Admin::updateSystemBlock(size_t index, uint16_t value)
{
    bool retVal = false;

    SYSTEM_BLOCK_ENTER(
        retVal = LessDb::update(0, static_cast<uint8_t>(Config::Section::system_t::SYSTEM_SETTINGS), index, value);)

    return retVal;
}

__attribute__((weak)) void database::Admin::customInitGlobal()
{
}

__attribute__((weak)) void database::Admin::customInitButtons()
{
}

__attribute__((weak)) void database::Admin::customInitEncoders()
{
}

__attribute__((weak)) void database::Admin::customInitAnalog()
{
}

__attribute__((weak)) void database::Admin::customInitLEDs()
{
}

__attribute__((weak)) void database::Admin::customInitDisplay()
{
}

__attribute__((weak)) void database::Admin::customInitTouchscreen()
{
}