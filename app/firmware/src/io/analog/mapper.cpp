/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mapper.h"

#include "util/conversion/conversion.h"

#include "zlibs/utils/misc/numeric.h"

using namespace opendeck::io::analog;

namespace
{
    constexpr opendeck::protocol::midi::MessageType INTERNAL_MSG_TO_MIDI_TYPE[static_cast<uint8_t>(Type::Count)] = {
        opendeck::protocol::midi::MessageType::ControlChange,         // PotentiometerControlChange
        opendeck::protocol::midi::MessageType::NoteOn,                // PotentiometerNote
        opendeck::protocol::midi::MessageType::NoteOn,                // Fsr
        opendeck::protocol::midi::MessageType::Invalid,               // Button
        opendeck::protocol::midi::MessageType::Nrpn7Bit,              // Nrpn7Bit
        opendeck::protocol::midi::MessageType::Nrpn14Bit,             // Nrpn14Bit
        opendeck::protocol::midi::MessageType::PitchBend,             // PitchBend
        opendeck::protocol::midi::MessageType::ControlChange14Bit,    // ControlChange14Bit
        opendeck::protocol::midi::MessageType::Invalid,               // Reserved
    };
}    // namespace

std::optional<Mapper::Result> Mapper::result(size_t index, uint16_t position)
{
    if (index >= Collection::size())
    {
        return {};
    }

    const auto info = read_database_info(index);

    if (info.type == Type::Button)
    {
        _last_value[index] = {
            .fsr_pressed = false,
            .value       = position,
            .max_value   = output_max_value(info),
            .valid       = true,
        };

        return button_result(info, position);
    }

    if (position > opendeck::io::analog::Filter::POSITION_MAX_VALUE)
    {
        return {};
    }

    const bool was_fsr_pressed  = _last_value[index].fsr_pressed;
    const auto mapped_value     = compute_value(position, info);
    const auto mapped_max_value = output_max_value(info);
    const bool fsr_pressed      = (info.type == Type::Fsr) && (mapped_value > 0);

    if (_last_value[index].valid &&
        (_last_value[index].value == mapped_value) &&
        (_last_value[index].max_value == mapped_max_value))
    {
        return {};
    }

    _last_value[index] = {
        .fsr_pressed = fsr_pressed,
        .value       = mapped_value,
        .max_value   = mapped_max_value,
        .valid       = true,
    };

    Result result = {};
    result.osc.emplace();
    fill_osc_signal(position, info, *result.osc);

    if ((info.type == Type::ControlChange14Bit) &&
        (midi_index(info) >= protocol::midi::CONTROL_CHANGE_14BIT_MAX_INDEX))
    {
        return result;
    }

    if (info.type != Type::Fsr)
    {
        result.midi.emplace();
        fill_midi_signal(*result.midi, info, mapped_value, info.midi_message);
        return result;
    }

    if (fsr_pressed)
    {
        if (!was_fsr_pressed)
        {
            result.midi.emplace();
            fill_midi_signal(*result.midi, info, mapped_value, info.midi_message);
        }

        return result;
    }

    if (!was_fsr_pressed)
    {
        return result;
    }

    result.midi.emplace();
    fill_midi_signal(*result.midi, info, 0, protocol::midi::MessageType::NoteOff);

    return result;
}

Mapper::Result Mapper::button_result(const DatabaseInfo& info, uint16_t value) const
{
    Result result = {};
    result.osc.emplace();
    result.midi.emplace();
    fill_osc_signal(value, info, *result.osc);
    fill_midi_signal(*result.midi, info, value, info.midi_message);

    return result;
}

std::optional<Mapper::Result> Mapper::last_result(size_t index) const
{
    if ((index >= Collection::size()) || !_last_value[index].valid)
    {
        return {};
    }

    const auto& last = _last_value[index];
    const auto  info = read_database_info(index);

    if (info.type == Type::Button)
    {
        return button_result(info, last.value);
    }

    Result result = {};
    result.osc.emplace();
    result.osc->source          = signaling::IoEventSource::Analog;
    result.osc->component_index = info.component_index;
    result.osc->int32_value     = static_cast<int32_t>(last.value);
    result.osc->float_value     = last.max_value != 0
                                      ? std::optional<float>(static_cast<float>(last.value) / static_cast<float>(last.max_value))
                                      : std::optional<float>(0.0F);
    result.osc->direction       = signaling::SignalDirection::Out;

    result.midi.emplace();
    fill_midi_signal(*result.midi, info, last.value, info.midi_message);

    if ((info.type == Type::Fsr) && !last.fsr_pressed)
    {
        fill_midi_signal(*result.midi, info, 0, protocol::midi::MessageType::NoteOff);
    }

    if ((info.type == Type::ControlChange14Bit) &&
        (midi_index(info) >= protocol::midi::CONTROL_CHANGE_14BIT_MAX_INDEX))
    {
        result.midi.reset();
    }

    return result;
}

Mapper::DatabaseInfo Mapper::read_database_info(size_t index) const
{
    DatabaseInfo info    = {};
    info.type            = static_cast<Type>(_database.read(database::Config::Section::Analog::Type, index));
    info.inverted        = _database.read(database::Config::Section::Analog::Invert, index);
    info.lower_limit     = _database.read(database::Config::Section::Analog::LowerLimit, index);
    info.upper_limit     = _database.read(database::Config::Section::Analog::UpperLimit, index);
    info.component_index = index;
    info.channel         = _database.read(database::Config::Section::Analog::Channel, index);
    info.midi_index      = _database.read(database::Config::Section::Analog::MidiId, index);
    info.midi_message    = INTERNAL_MSG_TO_MIDI_TYPE[static_cast<uint8_t>(info.type)];

    switch (info.type)
    {
    case Type::Nrpn14Bit:
    case Type::PitchBend:
    case Type::ControlChange14Bit:
        info.midi_max_value = protocol::midi::MAX_VALUE_14BIT;
        break;

    default:
        info.midi_max_value = protocol::midi::MAX_VALUE_7BIT;
        break;
    }

    return info;
}

uint16_t Mapper::output_max_value(const DatabaseInfo& info) const
{
    if (info.type == Type::Button)
    {
        return 1;
    }

    return info.midi_max_value;
}

void Mapper::reset(size_t index)
{
    if (index >= Collection::size())
    {
        return;
    }

    _last_value[index] = {};
}

void Mapper::fill_midi_signal(signaling::MidiIoSignal&    signal,
                              const DatabaseInfo&         info,
                              uint16_t                    value,
                              protocol::midi::MessageType message) const
{
    signal.source          = signaling::IoEventSource::Analog;
    signal.component_index = info.component_index;
    signal.channel         = info.channel;
    signal.index           = midi_index(info);
    signal.value           = value;
    signal.message         = message;
}

void Mapper::fill_osc_signal(uint16_t position, const DatabaseInfo& info, signaling::OscIoSignal& signal) const
{
    uint16_t mapped_value = 0;
    uint16_t max_value    = output_max_value(info);

    if (info.type == Type::Button)
    {
        mapped_value = position;
    }
    else
    {
        mapped_value = compute_value(position, info);
    }

    signal.source          = signaling::IoEventSource::Analog;
    signal.component_index = info.component_index;
    signal.int32_value     = static_cast<int32_t>(mapped_value);
    signal.float_value     = max_value != 0 ? std::optional<float>(static_cast<float>(mapped_value) / static_cast<float>(max_value))
                                            : std::optional<float>(0.0F);
    signal.direction       = signaling::SignalDirection::Out;
}

uint16_t Mapper::compute_value(uint16_t position, const DatabaseInfo& info) const
{
    if (info.type == Type::Button)
    {
        return position != 0 ? 1 : 0;
    }

    const auto max_value = output_max_value(info);

    uint16_t lower_limit = info.lower_limit;
    uint16_t upper_limit = info.upper_limit;

    if (is_7bit_type(info.type))
    {
        lower_limit = util::Conversion::Split14Bit(lower_limit).low();
        upper_limit = util::Conversion::Split14Bit(upper_limit).low();
    }

    uint32_t mapped = 0;

    if (lower_limit > upper_limit)
    {
        mapped = zlibs::utils::misc::map_range(static_cast<uint32_t>(position),
                                               static_cast<uint32_t>(0),
                                               static_cast<uint32_t>(opendeck::io::analog::Filter::POSITION_MAX_VALUE),
                                               static_cast<uint32_t>(upper_limit),
                                               static_cast<uint32_t>(lower_limit));

        if (!info.inverted)
        {
            mapped = upper_limit - (mapped - lower_limit);
        }
    }
    else
    {
        mapped = zlibs::utils::misc::map_range(static_cast<uint32_t>(position),
                                               static_cast<uint32_t>(0),
                                               static_cast<uint32_t>(opendeck::io::analog::Filter::POSITION_MAX_VALUE),
                                               static_cast<uint32_t>(lower_limit),
                                               static_cast<uint32_t>(upper_limit));

        if (info.inverted)
        {
            mapped = upper_limit - (mapped - lower_limit);
        }
    }

    return static_cast<uint16_t>(zlibs::utils::misc::constrain(mapped,
                                                               static_cast<uint32_t>(0),
                                                               static_cast<uint32_t>(max_value)));
}

uint16_t Mapper::midi_index(const DatabaseInfo& info) const
{
    if (is_7bit_type(info.type))
    {
        return util::Conversion::Split14Bit(info.midi_index).low();
    }

    return info.midi_index;
}

bool Mapper::is_7bit_type(Type type) const
{
    switch (type)
    {
    case Type::Nrpn14Bit:
    case Type::PitchBend:
    case Type::ControlChange14Bit:
        return false;

    default:
        return true;
    }
}
