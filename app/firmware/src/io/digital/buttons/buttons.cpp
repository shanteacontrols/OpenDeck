/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_BUTTONS

#include "buttons.h"
#include "system/config.h"
#include "util/conversion/conversion.h"
#include "util/configurable/configurable.h"

#include <zephyr/logging/log.h>

using namespace opendeck;
using namespace opendeck::io::buttons;
using namespace opendeck::protocol;

namespace
{
    LOG_MODULE_REGISTER(buttons, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

Buttons::Buttons(Hwa&      hwa,
                 Filter&   filter,
                 Mapper&   mapper,
                 Database& database)
    : _hwa(hwa)
    , _filter(filter)
    , _mapper(mapper)
    , _database(database)
{
    signaling::subscribe<signaling::MidiIoSignal>(
        [this](const signaling::MidiIoSignal& event)
        {
            if (is_frozen())
            {
                return;
            }

            if (event.source == signaling::IoEventSource::AnalogButton)
            {
                size_t index = event.component_index + Collection::start_index(GroupAnalogInputs);
                process_button(index, event.value);
                return;
            }

            if (event.source == signaling::IoEventSource::TouchscreenButton)
            {
                size_t index = event.component_index + Collection::start_index(GroupTouchscreenComponents);
                process_button(index, event.value);
            }
        });

    ConfigHandler.register_config(
        sys::Config::Block::Buttons,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sys_config_get(static_cast<sys::Config::Section::Button>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sys_config_set(static_cast<sys::Config::Section::Button>(section), index, value);
        });
}

bool Buttons::init()
{
    if (!_hwa.init())
    {
        return false;
    }

    for (size_t i = 0; i < Collection::size(); i++)
    {
        reset(i);
    }

    return true;
}

void Buttons::deinit()
{
}

size_t Buttons::refreshable_components() const
{
    return Collection::size(GroupDigitalInputs);
}

void Buttons::force_refresh(size_t start_index, size_t count)
{
    const auto total = Collection::size(GroupDigitalInputs);

    if (start_index >= total)
    {
        return;
    }

    const auto end = std::min(start_index + count, total);

    for (size_t i = start_index; i < end; i++)
    {
        if (const auto result = _mapper.refresh_result(i); result.has_value())
        {
            publish_result(result.value(), true);
        }
    }
}

void Buttons::process_state_changes()
{
    if (is_frozen())
    {
        return;
    }

    for (size_t i = 0; i < Collection::size(GroupDigitalInputs); i++)
    {
        auto current_state = state(i);

        if (!current_state.has_value())
        {
            continue;
        }

        bool filtered_state = current_state.value();

        if (!_filter.is_filtered(i, filtered_state))
        {
            continue;
        }

        process_button(i, filtered_state);
    }
}

void Buttons::process_button(size_t index, bool reading)
{
    if (const auto result = _mapper.result(index, reading); result.has_value())
    {
        publish_result(result.value());
    }
}

void Buttons::publish_result(const Result& result, bool ignore_freeze)
{
    if (is_frozen() && !ignore_freeze)
    {
        return;
    }

    if (result.osc.has_value())
    {
        signaling::publish(*result.osc);
    }

    if (result.midi.has_value())
    {
        signaling::publish(*result.midi);
    }

    if (result.system.has_value())
    {
        signaling::publish(*result.system);
    }
}

void Buttons::reset(size_t index)
{
    _mapper.reset(index);
}

std::optional<bool> Buttons::state(size_t index)
{
    // if encoder under this index is enabled, just return false state each time
    if (_database.read(database::Config::Section::Encoder::Enable, _hwa.button_to_encoder_index(index)))
    {
        return {};
    }

    return _hwa.state(index);
}

std::optional<uint8_t> Buttons::sys_config_get(sys::Config::Section::Button section, size_t index, uint16_t& value)
{
    uint32_t read_value = 0;

    auto result = _database.read(util::Conversion::sys_2_db_section(section), index, read_value)
                      ? sys::Config::Status::Ack
                      : sys::Config::Status::ErrorRead;

    value = read_value;

    return result;
}

std::optional<uint8_t> Buttons::sys_config_set(sys::Config::Section::Button section, size_t index, uint16_t value)
{
    auto result = _database.update(util::Conversion::sys_2_db_section(section), index, value)
                      ? sys::Config::Status::Ack
                      : sys::Config::Status::ErrorWrite;

    if (result == sys::Config::Status::Ack)
    {
        if (
            (section == sys::Config::Section::Button::Type) ||
            (section == sys::Config::Section::Button::MessageType))
        {
            reset(index);
        }
    }

    return result;
}

#endif
