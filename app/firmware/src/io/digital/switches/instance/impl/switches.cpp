/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/io/digital/switches/instance/impl/switches.h"
#include "firmware/src/system/shared/config.h"
#include "firmware/src/util/conversion/conversion.h"
#include "firmware/src/util/configurable/configurable.h"

#include <zephyr/logging/log.h>

using namespace opendeck::firmware::io::switches;
using namespace opendeck::firmware::protocol;
using namespace opendeck::firmware;

namespace
{
    LOG_MODULE_REGISTER(switches, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

Switches::Switches(Hwa&      hwa,
                   Filter&   filter,
                   Database& database)
    : _hwa(hwa)
    , _filter(filter)
    , _database(database)
    , _mapper(_database)
{
    _database.register_layout_init_provider(
        database::Config::Section::Switch::MidiId,
        [](size_t index) -> std::optional<uint32_t>
        {
            for (size_t group = 0; group < Collection::groups(); group++)
            {
                const auto start = Collection::start_index(group);
                const auto end   = start + Collection::size(group);

                if ((index >= start) && (index < end))
                {
                    return index - start;
                }
            }

            return {};
        });

    signaling::subscribe<signaling::MidiIoSignal>(
        [this](const signaling::MidiIoSignal& event)
        {
            if (is_frozen())
            {
                return;
            }

            if (event.source == signaling::IoEventSource::AnalogSwitch)
            {
                size_t index = event.component_index + Collection::start_index(GroupAnalogInputs);
                process_switch(index, event.value);
                return;
            }

            if (event.source == signaling::IoEventSource::TouchscreenSwitch)
            {
                size_t index = event.component_index + Collection::start_index(GroupTouchscreenComponents);
                process_switch(index, event.value);
            }
        });

    ConfigHandler.register_config(
        sys::Config::Block::Switches,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sys_config_get(static_cast<sys::Config::Section::Switch>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sys_config_set(static_cast<sys::Config::Section::Switch>(section), index, value);
        });
}

bool Switches::init()
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

void Switches::deinit()
{
}

size_t Switches::refreshable_components() const
{
    return Collection::size(GroupDigitalInputs);
}

void Switches::force_refresh(size_t start_index, size_t count)
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

void Switches::process_state_changes()
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

        process_switch(i, filtered_state);
    }
}

void Switches::process_switch(size_t index, bool reading)
{
    if (const auto result = _mapper.result(index, reading); result.has_value())
    {
        publish_result(result.value());
    }
}

void Switches::publish_result(const Result& result, bool ignore_freeze)
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

void Switches::reset(size_t index)
{
    _mapper.reset(index);
}

std::optional<bool> Switches::state(size_t index)
{
    // if encoder under this index is enabled, just return false state each time
    if (_database.read(database::Config::Section::Encoder::Enable, _hwa.switch_to_encoder_index(index)))
    {
        return {};
    }

    return _hwa.state(index);
}

std::optional<uint8_t> Switches::sys_config_get(sys::Config::Section::Switch section, size_t index, uint16_t& value)
{
    uint32_t read_value = 0;

    auto result = _database.read(util::Conversion::sys_2_db_section(section), index, read_value)
                      ? sys::Config::Status::Ack
                      : sys::Config::Status::ErrorRead;

    value = read_value;

    return result;
}

std::optional<uint8_t> Switches::sys_config_set(sys::Config::Section::Switch section, size_t index, uint16_t value)
{
    auto result = _database.update(util::Conversion::sys_2_db_section(section), index, value)
                      ? sys::Config::Status::Ack
                      : sys::Config::Status::ErrorWrite;

    if (result == sys::Config::Status::Ack)
    {
        if (
            (section == sys::Config::Section::Switch::Type) ||
            (section == sys::Config::Section::Switch::MessageType))
        {
            reset(index);
        }
    }

    return result;
}
