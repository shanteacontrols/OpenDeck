/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_TOUCHSCREEN

#include "touchscreen.h"
#include "io/outputs/common.h"
#include "util/conversion/conversion.h"
#include "util/configurable/configurable.h"

#include <zephyr/logging/log.h>

using namespace opendeck;
using namespace opendeck::io::touchscreen;

namespace
{
    LOG_MODULE_REGISTER(touchscreen, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

Touchscreen::Touchscreen(Hwa&      hwa,
                         Database& database)
    : _hwa(hwa)
    , _database(database)
    , _thread([&]()
              {
                  while (1)
                  {
                      wait_until_running();

                      if (wait_for_update(K_MSEC(THREAD_POLL_TIME_MS)))
                      {
                          process_update();
                          continue;
                      }

                      if (is_initialized())
                      {
                          process_update();
                      }
                  }
              })
{
    k_sem_init(&_update_semaphore, 0, K_SEM_MAX_LIMIT);

    signaling::subscribe<signaling::OscIoSignal>(
        [this](const signaling::OscIoSignal& signal)
        {
            if (is_frozen())
            {
                return;
            }

            if (signal.source != signaling::IoEventSource::Output)
            {
                return;
            }

            if (signal.direction != signaling::SignalDirection::Out)
            {
                return;
            }

            constexpr size_t TOUCHSCREEN_OUTPUT_START = outputs::Collection::start_index(outputs::GroupTouchscreenComponents);

            if (signal.component_index < TOUCHSCREEN_OUTPUT_START)
            {
                return;
            }

            const size_t touchscreen_index = signal.component_index - TOUCHSCREEN_OUTPUT_START;

            if (touchscreen_index >= Collection::size())
            {
                return;
            }

            set_icon_state(touchscreen_index, signal.int32_value.value_or(0) != 0);
            request_update();
        });

    signaling::subscribe<signaling::SystemSignal>(
        [this](const signaling::SystemSignal& event)
        {
            switch (event.system_event)
            {
            case signaling::SystemEvent::PresetChanged:
            {
                if (!init())
                {
                    deinit_model();
                }

                if (!is_frozen())
                {
                    request_update();
                }
            }
            break;

            default:
                break;
            }
        });

    ConfigHandler.register_config(
        sys::Config::Block::Touchscreen,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sys_config_get(static_cast<sys::Config::Section::Touchscreen>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sys_config_set(static_cast<sys::Config::Section::Touchscreen>(section), index, value);
        });
}

Touchscreen::~Touchscreen()
{
    stop_thread();

    for (size_t i = 0; i < static_cast<size_t>(ModelType::Count); i++)
    {
        models.at(i) = nullptr;
    }
}

bool Touchscreen::init()
{
    if (_database.read(database::Config::Section::Touchscreen::Setting, touchscreen::Setting::Enable))
    {
        const zlibs::utils::misc::LockGuard lock(_state_mutex);
        auto                                db_model = static_cast<ModelType>(_database.read(database::Config::Section::Touchscreen::Setting,
                                                                                             touchscreen::Setting::Model));

        if (_initialized)
        {
            if (db_model == _active_model)
            {
                // nothing to do, same model already _initialized
                return true;
            }

            if (!deinit_model())
            {
                return false;
            }
        }

        _active_model = db_model;
        auto instance = model_instance(_active_model);

        if (instance != nullptr)
        {
            _initialized = instance->init();
        }

        if (_initialized)
        {
            _thread.run();
            set_screen(_database.read(database::Config::Section::Touchscreen::Setting,
                                      touchscreen::Setting::InitialScreen));

            set_brightness(static_cast<Brightness>(_database.read(database::Config::Section::Touchscreen::Setting,
                                                                  touchscreen::Setting::Brightness)));

            return true;
        }

        return false;
    }

    return false;
}

bool Touchscreen::deinit_model()
{
    const zlibs::utils::misc::LockGuard lock(_state_mutex);

    if (!_initialized)
    {
        return true;    // nothing to do
    }

    auto ptr = model_instance(_active_model);

    if (ptr == nullptr)
    {
        return false;
    }

    if (ptr->deinit())
    {
        _initialized = false;
        return true;
    }

    return false;
}

void Touchscreen::deinit()
{
    stop_thread();
}

void Touchscreen::stop_thread()
{
    _thread.destroy();
}

void Touchscreen::register_model(ModelType model, Model* instance)
{
    models[static_cast<size_t>(model)] = instance;
}

void Touchscreen::request_update()
{
    if (is_frozen())
    {
        return;
    }

    k_sem_give(&_update_semaphore);
}

bool Touchscreen::wait_for_update(k_timeout_t timeout)
{
    return k_sem_take(&_update_semaphore, timeout) == 0;
}

void Touchscreen::process_update()
{
    if (is_frozen())
    {
        return;
    }

    Data    data  = {};
    TsEvent event = TsEvent::None;

    {
        const zlibs::utils::misc::LockGuard lock(_state_mutex);

        if (!_initialized)
        {
            return;
        }

        auto ptr = model_instance(_active_model);

        if (ptr == nullptr)
        {
            return;
        }

        event = ptr->update(data);
    }

    switch (event)
    {
    case TsEvent::Switch:
    {
        process_switch(data.switch_index, data.switch_state);
    }
    break;

    default:
        break;
    }
}

void Touchscreen::set_screen(size_t index)
{
    if (!is_initialized())
    {
        return;
    }

    auto ptr = model_instance(_active_model);

    if (ptr == nullptr)
    {
        return;
    }

    ptr->set_screen(index);
    _active_screen_id = index;
    screen_change_handler(index);
}

size_t Touchscreen::active_screen()
{
    return _active_screen_id;
}

void Touchscreen::set_icon_state(size_t index, bool state)
{
    if (!is_initialized())
    {
        return;
    }

    if (index >= Collection::size())
    {
        return;
    }

    auto ptr = model_instance(_active_model);

    if (ptr == nullptr)
    {
        return;
    }

    Icon icon       = {};
    icon.on_screen  = _database.read(database::Config::Section::Touchscreen::OnScreen, index);
    icon.off_screen = _database.read(database::Config::Section::Touchscreen::OffScreen, index);

    if (icon.on_screen == icon.off_screen)
    {
        return;    // invalid screen indexes
    }

    if ((_active_screen_id != icon.on_screen) && (_active_screen_id != icon.off_screen))
    {
        return;    // don't allow setting icon on wrong screen
    }

    icon.x_pos  = _database.read(database::Config::Section::Touchscreen::XPos, index);
    icon.y_pos  = _database.read(database::Config::Section::Touchscreen::YPos, index);
    icon.width  = _database.read(database::Config::Section::Touchscreen::Width, index);
    icon.height = _database.read(database::Config::Section::Touchscreen::Height, index);

    ptr->set_icon_state(icon, state);
}

void Touchscreen::process_switch(const size_t switch_index, const bool state)
{
    bool   change_screen = false;
    size_t new_screen    = 0;

    if (_database.read(database::Config::Section::Touchscreen::PageSwitchEnabled, switch_index))
    {
        change_screen = true;
        new_screen    = _database.read(database::Config::Section::Touchscreen::PageSwitchIndex, switch_index);
    }

    switch_handler(switch_index, state);

    // if the switch should change screen, change it immediately
    // this will result in switch never sending off state so do it manually first
    if (change_screen)
    {
        switch_handler(switch_index, false);
        set_screen(new_screen);
    }
}

bool Touchscreen::set_brightness(Brightness brightness)
{
    if (!is_initialized())
    {
        return false;
    }

    auto ptr = model_instance(_active_model);

    if (ptr == nullptr)
    {
        return false;
    }

    return ptr->set_brightness(brightness);
}

bool Touchscreen::is_initialized() const
{
    return _initialized;
}

Model* Touchscreen::model_instance(ModelType model)
{
    return models[static_cast<size_t>(model)];
}

void Touchscreen::switch_handler(size_t index, bool state)
{
    signaling::MidiIoSignal signal = {};
    signal.source                  = signaling::IoEventSource::TouchscreenSwitch;
    signal.component_index         = index;
    signal.value                   = state;

    signaling::publish(signal);
}

void Touchscreen::screen_change_handler(size_t index)
{
    signaling::TouchscreenScreenChangedSignal signal = {};
    signal.screen_index                              = index;

    signaling::publish(signal);
}

std::optional<uint8_t> Touchscreen::sys_config_get(sys::Config::Section::Touchscreen section, size_t index, uint16_t& value)
{
    if (!is_initialized() && _hwa.allocated(io::common::Allocatable::Interface::Uart))
    {
        return sys::Config::Status::SerialPeripheralAllocatedError;
    }

    uint32_t read_value = 0;

    auto result = _database.read(util::Conversion::sys_2_db_section(section), index, read_value)
                      ? sys::Config::Status::Ack
                      : sys::Config::Status::ErrorRead;

    value = read_value;
    return result;
}

std::optional<uint8_t> Touchscreen::sys_config_set(sys::Config::Section::Touchscreen section, size_t index, uint16_t value)
{
    if (!is_initialized() && _hwa.allocated(io::common::Allocatable::Interface::Uart))
    {
        return sys::Config::Status::SerialPeripheralAllocatedError;
    }

    auto init_action = common::InitAction::AsIs;
    bool write_to_db = true;

    switch (section)
    {
    case sys::Config::Section::Touchscreen::Setting:
    {
        switch (index)
        {
        case static_cast<size_t>(Setting::Enable):
        {
            if (value)
            {
                init_action = common::InitAction::Init;
            }
            else
            {
                init_action = common::InitAction::DeInit;
            }
        }
        break;

        case static_cast<size_t>(Setting::Model):
        {
            if (value >= static_cast<size_t>(ModelType::Count))
            {
                return sys::Config::Status::ErrorNewValue;
            }

            init_action = common::InitAction::Init;
        }
        break;

        case static_cast<size_t>(Setting::Brightness):
        {
            if (is_initialized())
            {
                if (!set_brightness(static_cast<Brightness>(value)))
                {
                    return sys::Config::Status::ErrorWrite;
                }
            }
        }
        break;

        case static_cast<size_t>(Setting::InitialScreen):
        {
            if (is_initialized())
            {
                set_screen(value);
            }
        }
        break;

        default:
            break;
        }
    }
    break;

    default:
        break;
    }

    bool result = true;

    if (write_to_db)
    {
        result = _database.update(util::Conversion::sys_2_db_section(section), index, value);
    }

    if (result)
    {
        if (init_action == common::InitAction::Init)
        {
            init();
        }
        else if (init_action == common::InitAction::DeInit)
        {
            deinit_model();
        }

        return sys::Config::Status::Ack;
    }

    return sys::Config::Status::ErrorWrite;
}

#endif
