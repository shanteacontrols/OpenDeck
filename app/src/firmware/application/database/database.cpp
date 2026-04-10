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

#include "zlibs/utils/misc/numeric.h"

#include <zephyr/logging/log.h>

#include <inttypes.h>

using namespace zlibs::utils::lessdb;

namespace
{
    LOG_MODULE_REGISTER(database, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

database::Admin::Admin(Hwa&    hwa,
                       Layout& layout)
    : LessDb::LessDb(hwa)
    , _hwa(hwa)
    , _layout(layout)
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
    LOG_INF("Initializing database");

    _handlers = &handlers;

    if (!LessDb::init())
    {
        return false;
    }

    const uint32_t commonLayoutSize = _layout.commonLayoutSize();

    if (!LessDb::set_layout(_layout.commonLayout()))
    {
        LOG_ERR("Setting common layout failed");
        return false;
    }

    _presetDataStartAddress = commonLayoutSize;
    _activeContext          = Context::COMMON;

    if (!LessDb::set_layout(_layout.presetLayout(), _presetDataStartAddress))
    {
        LOG_ERR("Setting preset layout failed");
        return false;
    }

    _presetLayoutSize = _layout.presetLayoutSize();
    _activeContext    = Context::PRESET;

    // get theoretical maximum of presets
    _supportedPresets = _presetLayoutSize ? (LessDb::db_size() - commonLayoutSize) / _presetLayoutSize : 0;

    // limit by hardcoded limit
    _supportedPresets = zlibs::utils::misc::constrain(_supportedPresets,
                                                      static_cast<size_t>(0),
                                                      Config::MAX_PRESETS);

    bool retVal = true;

    if (!isSignatureValid())
    {
        LOG_WRN("Signature invalid");

        bool restoredFromSnapshot = false;

        if (_hwa.hasFactorySnapshot())
        {
            LOG_INF("Restoring snapshot from factory page");

            retVal = _hwa.restoreFactorySnapshot();

            if (retVal)
            {
                restoredFromSnapshot = isSignatureValid();
                retVal               = restoredFromSnapshot;
            }
        }

        if (restoredFromSnapshot)
        {
            LOG_INF("Snapshot restored, loading data");
            retVal = loadStoredPresetState();
        }
        else
        {
            LOG_WRN("Restoring from snapshot failed or snapshot is missing");
            retVal = factoryReset();
        }
    }
    else
    {
        LOG_INF("Loading data");
        retVal = loadStoredPresetState();
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
    LOG_INF("Performing factory reset");

    if (_handlers != nullptr)
    {
        _handlers->factoryResetStart();
    }

    if (!clear())
    {
        return false;
    }

    if (_hwa.hasFactorySnapshot())
    {
        if (_hwa.restoreFactorySnapshot() && isSignatureValid())
        {
            if (!loadStoredPresetState())
            {
                return false;
            }

            if (_handlers != nullptr)
            {
                _handlers->factoryResetDone();
            }

            return true;
        }

        LOG_WRN("Factory snapshot restore failed, regenerating defaults");
    }

    if (!initializeDefaultData())
    {
        return false;
    }

    if (!_hwa.storeFactorySnapshot())
    {
        LOG_WRN("Failed to store factory snapshot");
    }

    if (_handlers != nullptr)
    {
        _handlers->factoryResetDone();
    }

    return true;
}

bool database::Admin::loadStoredPresetState()
{
    _activePreset = readCommonBlock(static_cast<size_t>(Config::commonSetting_t::ACTIVE_PRESET));

    if (getPresetPreserveState())
    {
        // don't write anything to database in this case - setup preset only internally
        return setPresetInternal(_activePreset);
    }

    if (_activePreset != 0)
    {
        // preset preservation is not set which means preset must be 0
        // in this case it is not so overwrite it with 0
        return setPreset(0);
    }

    return setPresetInternal(0);
}

bool database::Admin::initializeDefaultData()
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

    if (!setSignature())
    {
        return false;
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

uint8_t database::Admin::currentPreset()
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
    uint16_t storedSignature = readCommonBlock(static_cast<size_t>(Config::commonSetting_t::UID));

    return signature() == storedSignature;
}

uint16_t database::Admin::signature() const
{
    return static_cast<uint16_t>(_layout.presetUid() ^
                                 static_cast<uint16_t>(PROJECT_TARGET_UID) ^
                                 static_cast<uint16_t>(_supportedPresets));
}

bool database::Admin::setSignature()
{
    return updateCommonBlock(static_cast<size_t>(Config::commonSetting_t::UID), signature());
}

std::optional<uint8_t> database::Admin::sysConfigGet(sys::Config::Section::global_t section, size_t index, uint16_t& value)
{
    if (section != sys::Config::Section::global_t::SYSTEM_SETTINGS)
    {
        return {};
    }

    if (index >= static_cast<uint8_t>(Config::commonSetting_t::CUSTOM_COMMON_SETTING_START))
    {
        return {};
    }

    uint32_t readValue = 0;
    uint8_t  result    = sys::Config::Status::ERROR_READ;

    auto setting = static_cast<Config::commonSetting_t>(index);

    switch (setting)
    {
    case Config::commonSetting_t::ACTIVE_PRESET:
    {
        readValue = currentPreset();
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
        return {};
    }

    if (index >= static_cast<uint8_t>(Config::commonSetting_t::CUSTOM_COMMON_SETTING_START))
    {
        return {};
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
        return {};
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
