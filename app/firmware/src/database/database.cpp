/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/util/conversion/conversion.h"
#include "firmware/src/util/configurable/configurable.h"
#include "firmware/src/database/layout.h"

#include <zephyr/logging/log.h>

#include <cinttypes>

using namespace opendeck;
using namespace zlibs::utils::lessdb;

namespace
{
    LOG_MODULE_REGISTER(database, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

database::Admin::Admin(Hwa& hwa)
    : LessDb::LessDb(hwa)
    , _hwa(hwa)
{
    ConfigHandler.register_config(
        sys::Config::Block::Global,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sys_config_get(static_cast<sys::Config::Section::Global>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sys_config_set(static_cast<sys::Config::Section::Global>(section), index, value);
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

    constexpr uint32_t COMMON_LAYOUT_SIZE = AppLayout::common_layout_size();

    if (!LessDb::set_layout(AppLayout::common_layout()))
    {
        LOG_ERR("Setting common layout failed");
        return false;
    }

    _preset_data_start_address = COMMON_LAYOUT_SIZE;
    _active_context            = Context::Common;

    if (!LessDb::set_layout(AppLayout::preset_layout(), _preset_data_start_address))
    {
        LOG_ERR("Setting preset layout failed");
        return false;
    }

    _preset_layout_size = AppLayout::preset_layout_size();
    _active_context     = Context::Preset;

    _supported_presets = AppLayout::supported_preset_count_for(LessDb::address_count());

    bool ret_val = true;

    if (!is_signature_valid())
    {
        LOG_WRN("Signature invalid");

        bool restored_from_snapshot = false;

        if (_hwa.has_factory_snapshot())
        {
            LOG_INF("Restoring snapshot from factory page");

            ret_val = _hwa.restore_factory_snapshot();

            if (ret_val)
            {
                restored_from_snapshot = is_signature_valid();
            }
        }

        if (restored_from_snapshot)
        {
            LOG_INF("Snapshot restored, loading data");
            ret_val = load_stored_preset_state();
        }
        else
        {
            LOG_WRN("Restoring from snapshot failed or snapshot is missing");
            ret_val = factory_reset();
        }
    }
    else
    {
        LOG_INF("Loading data");
        ret_val = load_stored_preset_state();
    }

    if (ret_val)
    {
        _initialized = true;

        if (_handlers != nullptr)
        {
            _handlers->initialized();
        }
    }

    return ret_val;
}

bool database::Admin::is_initialized()
{
    return _initialized;
}

bool database::Admin::factory_reset()
{
    LOG_INF("Performing factory reset");

    if (_handlers != nullptr)
    {
        _handlers->factory_reset_start();
    }

    if (!clear())
    {
        return false;
    }

    if (_hwa.has_factory_snapshot())
    {
        if (_hwa.restore_factory_snapshot() && is_signature_valid())
        {
            if (!load_stored_preset_state())
            {
                return false;
            }

            if (_handlers != nullptr)
            {
                _handlers->factory_reset_done();
            }

            return true;
        }

        LOG_WRN("Factory snapshot restore failed, regenerating defaults");
    }

    if (!initialize_default_data())
    {
        return false;
    }

    if (!_hwa.store_factory_snapshot())
    {
        LOG_WRN("Failed to store factory snapshot");
    }

    if (_handlers != nullptr)
    {
        _handlers->factory_reset_done();
    }

    return true;
}

bool database::Admin::load_stored_preset_state()
{
    _active_preset = read_common_block(Config::Section::Common::CommonSettings,
                                       static_cast<size_t>(Config::CommonSetting::ActivePreset));

    if (preset_preserve_state())
    {
        // don't write anything to database in this case - setup preset only internally
        return set_preset_internal(_active_preset);
    }

    if (_active_preset != 0)
    {
        // preset preservation is not set which means preset must be 0
        // in this case it is not so overwrite it with 0
        return set_preset(0);
    }

    return set_preset_internal(0);
}

bool database::Admin::initialize_default_data()
{
    if (!select_common())
    {
        return false;
    }

    if (!init_data(FactoryResetType::Full))
    {
        return false;
    }

    for (size_t i = _supported_presets; i-- > 0;)
    {
        if (!set_preset_internal(i))
        {
            return false;
        }

        if (!init_data(FactoryResetType::Full))
        {
            return false;
        }

        // perform custom init as well
        custom_init_global();
        custom_init_switches();
        custom_init_encoders();
        custom_init_analog();
        custom_init_outputs();
        custom_init_display();
        custom_init_touchscreen();
    }

    if (!set_preset_preserve_state(false))
    {
        return false;
    }

    if (!set_signature())
    {
        return false;
    }

    return true;
}

bool database::Admin::set_preset(uint8_t preset)
{
    if (preset >= _supported_presets)
    {
        return false;
    }

    if (!select_preset(preset))
    {
        return false;
    }

    auto ret_val = update_common_block(Config::Section::Common::CommonSettings,
                                       static_cast<size_t>(Config::CommonSetting::ActivePreset),
                                       preset);

    if (ret_val)
    {
        if (_handlers != nullptr)
        {
            _handlers->preset_change(preset);
        }
    }

    return ret_val;
}

bool database::Admin::set_preset_internal(uint8_t preset)
{
    return select_preset(preset);
}

bool database::Admin::select_common()
{
    if (_active_context == Context::Common)
    {
        return true;
    }

    if (!LessDb::set_layout(AppLayout::common_layout(), 0))
    {
        return false;
    }

    _active_context = Context::Common;
    return true;
}

bool database::Admin::select_preset(uint8_t preset)
{
    if (preset >= _supported_presets)
    {
        return false;
    }

    if ((_active_context == Context::Preset) && (_active_preset == preset))
    {
        return true;
    }

    if (!LessDb::set_layout(AppLayout::preset_layout(), preset_start_address(preset)))
    {
        return false;
    }

    _active_preset  = preset;
    _active_context = Context::Preset;

    return true;
}

uint32_t database::Admin::preset_start_address(uint8_t preset) const
{
    return _preset_data_start_address + (_preset_layout_size * preset);
}

uint8_t database::Admin::current_preset()
{
    return _active_preset;
}

uint8_t database::Admin::supported_presets()
{
    return _supported_presets;
}

bool database::Admin::set_preset_preserve_state(bool state)
{
    return update_common_block(Config::Section::Common::CommonSettings,
                               static_cast<size_t>(Config::CommonSetting::PresetPreserve),
                               state);
}

bool database::Admin::preset_preserve_state()
{
    return read_common_block(Config::Section::Common::CommonSettings,
                             static_cast<size_t>(Config::CommonSetting::PresetPreserve));
}

bool database::Admin::is_signature_valid()
{
    uint16_t stored_signature = read_common_block(Config::Section::Common::CommonSettings,
                                                  static_cast<size_t>(Config::CommonSetting::Uid));

    return signature() == stored_signature;
}

uint16_t database::Admin::signature() const
{
    return static_cast<uint16_t>(AppLayout::common_uid() ^
                                 AppLayout::preset_uid() ^
                                 static_cast<uint16_t>(OPENDECK_TARGET_UID) ^
                                 static_cast<uint16_t>(_supported_presets));
}

bool database::Admin::set_signature()
{
    return update_common_block(Config::Section::Common::CommonSettings,
                               static_cast<size_t>(Config::CommonSetting::Uid),
                               signature());
}

std::optional<uint8_t> database::Admin::sys_config_get(sys::Config::Section::Global section, size_t index, uint16_t& value)
{
    if (section != sys::Config::Section::Global::SystemSettings)
    {
        return {};
    }

    if (index >= static_cast<uint8_t>(Config::CommonSetting::CustomCommonSettingStart))
    {
        return {};
    }

    uint32_t read_value = 0;
    uint8_t  result     = sys::Config::Status::ErrorRead;

    auto setting = static_cast<Config::CommonSetting>(index);

    switch (setting)
    {
    case Config::CommonSetting::ActivePreset:
    {
        read_value = current_preset();
        result     = sys::Config::Status::Ack;
    }
    break;

    case Config::CommonSetting::PresetPreserve:
    {
        read_value = preset_preserve_state();
        result     = sys::Config::Status::Ack;
    }
    break;

    default:
        break;
    }

    value = read_value;
    return result;
}

std::optional<uint8_t> database::Admin::sys_config_set(sys::Config::Section::Global section, size_t index, uint16_t value)
{
    if (section != sys::Config::Section::Global::SystemSettings)
    {
        return {};
    }

    if (index >= static_cast<uint8_t>(Config::CommonSetting::CustomCommonSettingStart))
    {
        return {};
    }

    uint8_t result  = sys::Config::Status::ErrorWrite;
    auto    setting = static_cast<Config::CommonSetting>(index);

    switch (setting)
    {
    case Config::CommonSetting::ActivePreset:
    {
        if (value < supported_presets())
        {
            set_preset(value);
            result = sys::Config::Status::Ack;
        }
        else
        {
            result = sys::Config::Status::ErrorNotSupported;
        }
    }
    break;

    case Config::CommonSetting::PresetPreserve:
    {
        if ((value <= 1) && (value >= 0))
        {
            set_preset_preserve_state(value);
            result = sys::Config::Status::Ack;
        }
    }
    break;

    default:
        return {};
    }

    return result;
}

uint16_t database::Admin::read_common_block(Config::Section::Common section, size_t index)
{
    if (!select_common())
    {
        return 0;
    }

    return LessDb::read(0,
                        static_cast<uint8_t>(section),
                        index)
        .value_or(0);
}

bool database::Admin::read_common_block(Config::Section::Common section, size_t index, uint32_t& value)
{
    if (!select_common())
    {
        return false;
    }

    auto read_value = LessDb::read(0,
                                   static_cast<uint8_t>(section),
                                   index);

    if (!read_value)
    {
        return false;
    }

    value = *read_value;
    return true;
}

bool database::Admin::update_common_block(Config::Section::Common section, size_t index, uint16_t value)
{
    if (!select_common())
    {
        return false;
    }

    return LessDb::update(0,
                          static_cast<uint8_t>(section),
                          index,
                          value);
}
