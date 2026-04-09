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

    const uint32_t commonLayoutSize = _layout.commonLayoutSize();

    if (!LessDb::set_layout(_layout.commonLayout()))
    {
        return false;
    }

    _presetDataStartAddress = commonLayoutSize;
    _activeContext          = Context::COMMON;

    if (!LessDb::set_layout(_layout.presetLayout(), _presetDataStartAddress))
    {
        return false;
    }

    _presetLayoutSize = _layout.presetLayoutSize();
    _activeContext    = Context::PRESET;

    // get theoretical maximum of presets
    _supportedPresets = _presetLayoutSize ? (LessDb::db_size() - commonLayoutSize) / _presetLayoutSize : 0;

    // limit by hardcoded limit
    _supportedPresets = core::util::CONSTRAIN(_supportedPresets,
                                              static_cast<size_t>(0),
                                              Config::MAX_PRESETS);

    _uid = _layout.presetUid();

    bool retVal = true;

    if (!isSignatureValid())
    {
        retVal = factoryReset();
    }
    else
    {
        _activePreset = readCommonBlock(static_cast<size_t>(Config::commonSetting_t::ACTIVE_PRESET));

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
        if (!selectCommon())
        {
            return false;
        }

        if (!init_data(FactoryResetType::Full))
        {
            return false;
        }

        for (size_t i = _supportedPresets; i-- > 0;)
        {
            if (!setPresetInternal(i))
            {
                return false;
            }

            if (!init_data(FactoryResetType::Full))
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

bool database::Admin::setPreset(uint8_t preset)
{
    if (preset >= _supportedPresets)
    {
        return false;
    }

    if (!selectPreset(preset))
    {
        return false;
    }

    auto retVal = updateCommonBlock(static_cast<size_t>(Config::commonSetting_t::ACTIVE_PRESET),
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

bool database::Admin::setPresetInternal(uint8_t preset)
{
    return selectPreset(preset);
}

bool database::Admin::selectCommon()
{
    if (_activeContext == Context::COMMON)
    {
        return true;
    }

    if (!LessDb::set_layout(_layout.commonLayout(), 0))
    {
        return false;
    }

    _activeContext = Context::COMMON;
    return true;
}

bool database::Admin::selectPreset(uint8_t preset)
{
    if (preset >= _supportedPresets)
    {
        return false;
    }

    if ((_activeContext == Context::PRESET) && (_activePreset == preset))
    {
        return true;
    }

    if (!LessDb::set_layout(_layout.presetLayout(), presetStartAddress(preset)))
    {
        return false;
    }

    _activePreset  = preset;
    _activeContext = Context::PRESET;

    return true;
}

uint32_t database::Admin::presetStartAddress(uint8_t preset) const
{
    return _presetDataStartAddress + (_presetLayoutSize * preset);
}

uint8_t database::Admin::getPreset()
{
    return _activePreset;
}

uint8_t database::Admin::getSupportedPresets()
{
    return _supportedPresets;
}

bool database::Admin::setPresetPreserveState(bool state)
{
    return updateCommonBlock(static_cast<size_t>(Config::commonSetting_t::PRESET_PRESERVE),
                             state);
}

bool database::Admin::getPresetPreserveState()
{
    return readCommonBlock(static_cast<size_t>(Config::commonSetting_t::PRESET_PRESERVE));
}

bool database::Admin::isSignatureValid()
{
    uint16_t signature = readCommonBlock(static_cast<size_t>(Config::commonSetting_t::UID));

    return _uid == signature;
}

bool database::Admin::setUID()
{
    return updateCommonBlock(static_cast<size_t>(Config::commonSetting_t::UID), _uid);
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

    if (index >= static_cast<uint8_t>(Config::commonSetting_t::CUSTOM_COMMON_SETTING_START))
    {
        return std::nullopt;
    }

    uint32_t readValue = 0;
    uint8_t  result    = sys::Config::Status::ERROR_READ;

    auto setting = static_cast<Config::commonSetting_t>(index);

    switch (setting)
    {
    case Config::commonSetting_t::ACTIVE_PRESET:
    {
        readValue = getPreset();
        result    = sys::Config::Status::ACK;
    }
    break;

    case Config::commonSetting_t::PRESET_PRESERVE:
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

    if (index >= static_cast<uint8_t>(Config::commonSetting_t::CUSTOM_COMMON_SETTING_START))
    {
        return std::nullopt;
    }

    uint8_t result  = sys::Config::Status::ERROR_WRITE;
    auto    setting = static_cast<Config::commonSetting_t>(index);

    switch (setting)
    {
    case Config::commonSetting_t::ACTIVE_PRESET:
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

    case Config::commonSetting_t::PRESET_PRESERVE:
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

uint16_t database::Admin::readCommonBlock(size_t index)
{
    if (!selectCommon())
    {
        return 0;
    }

    return LessDb::read(0,
                        static_cast<uint8_t>(Config::Section::common_t::COMMON_SETTINGS),
                        index)
        .value_or(0);
}

bool database::Admin::updateCommonBlock(size_t index, uint16_t value)
{
    if (!selectCommon())
    {
        return false;
    }

    return LessDb::update(0,
                          static_cast<uint8_t>(Config::Section::common_t::COMMON_SETTINGS),
                          index,
                          value);
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
