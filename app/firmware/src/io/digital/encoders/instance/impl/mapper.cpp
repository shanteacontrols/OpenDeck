/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/io/digital/encoders/instance/impl/mapper.h"
#include "firmware/src/global/bpm.h"

#include "zlibs/utils/misc/numeric.h"

using namespace opendeck::io::encoders;
using namespace opendeck::protocol;

namespace
{
    constexpr std::array<opendeck::protocol::midi::MessageType, static_cast<uint8_t>(Type::Count)> INTERNAL_MSG_TO_MIDI_TYPE = {
        opendeck::protocol::midi::MessageType::ControlChange,         // ControlChange7fh01h
        opendeck::protocol::midi::MessageType::ControlChange,         // ControlChange3fh41h
        opendeck::protocol::midi::MessageType::ProgramChange,         // ProgramChange
        opendeck::protocol::midi::MessageType::ControlChange,         // ControlChange
        opendeck::protocol::midi::MessageType::Invalid,               // PresetChange
        opendeck::protocol::midi::MessageType::PitchBend,             // PitchBend
        opendeck::protocol::midi::MessageType::Nrpn7Bit,              // Nrpn7Bit
        opendeck::protocol::midi::MessageType::Nrpn14Bit,             // Nrpn14Bit
        opendeck::protocol::midi::MessageType::ControlChange14Bit,    // ControlChange14Bit
        opendeck::protocol::midi::MessageType::ControlChange,         // ControlChange41h01h
        opendeck::protocol::midi::MessageType::Invalid,               // BpmChange
        opendeck::protocol::midi::MessageType::NoteOn,                // SingleNoteVariableVal
        opendeck::protocol::midi::MessageType::NoteOn,                // SingleNoteFixedValBothDir
        opendeck::protocol::midi::MessageType::NoteOn,                // SingleNoteFixedValOneDir0OtherDir
        opendeck::protocol::midi::MessageType::NoteOn,                // TwoNoteFixedValBothDir
    };
}

std::optional<Mapper::Result> Mapper::result(size_t index, Position position, uint8_t steps)
{
    if ((index >= Collection::size()) || (position == Position::Stopped))
    {
        return {};
    }

    auto info = read_database_info(index, position);

    if (info.inverted)
    {
        position = position == Position::Ccw ? Position::Cw : Position::Ccw;
    }

    Result result = {};
    bool   send   = true;
    auto   value  = static_cast<uint16_t>(_value[index]);

    switch (info.type)
    {
    case Type::ControlChange7fh01h:
        value = VAL_CONTROL_CHANGE_7FH01H[static_cast<uint8_t>(position)];
        break;

    case Type::ControlChange3fh41h:
        value = VAL_CONTROL_CHANGE_3FH41H[static_cast<uint8_t>(position)];
        break;

    case Type::ControlChange41h01h:
        value = VAL_CONTROL_CHANGE_41H01H[static_cast<uint8_t>(position)];
        break;

    case Type::ProgramChange:
    {
        if (position == Position::Ccw)
        {
            if (!MidiProgram.increment_program(info.channel, 1))
            {
                return {};
            }
        }
        else
        {
            if (!MidiProgram.decrement_program(info.channel, 1))
            {
                return {};
            }
        }

        value         = MidiProgram.program(info.channel);
        _value[index] = static_cast<int16_t>(value);
    }
    break;

    case Type::ControlChange:
    case Type::PitchBend:
    case Type::Nrpn7Bit:
    case Type::Nrpn14Bit:
    case Type::ControlChange14Bit:
    case Type::SingleNoteVariableVal:
    {
        uint8_t    effective_steps = steps;
        const bool use_14bit       = ((info.type == Type::PitchBend) || (info.type == Type::Nrpn14Bit) || (info.type == Type::ControlChange14Bit));

        if (use_14bit && (effective_steps > 1))
        {
            effective_steps <<= 2;
        }

        if (position == Position::Ccw)
        {
            _value[index] = ValueIncDecMidi7Bit::decrement(_value[index],
                                                           effective_steps,
                                                           ValueIncDecMidi7Bit::Type::Edge);
        }
        else
        {
            switch (info.type)
            {
            case Type::ControlChange:
            case Type::Nrpn7Bit:
            case Type::SingleNoteVariableVal:
                _value[index] = ValueIncDecMidi7Bit::increment(_value[index],
                                                               effective_steps,
                                                               ValueIncDecMidi7Bit::Type::Edge);
                break;

            case Type::PitchBend:
            case Type::Nrpn14Bit:
            case Type::ControlChange14Bit:
            {
                const auto incremented_value = ValueIncDecMidi14Bit::increment(static_cast<uint16_t>(_value[index]),
                                                                               effective_steps,
                                                                               ValueIncDecMidi14Bit::Type::Edge);

                _value[index] = static_cast<int16_t>(incremented_value);
            }
            break;

            default:
                break;
            }
        }

        value         = zlibs::utils::misc::constrain(static_cast<uint16_t>(_value[index]), info.lower_limit, info.upper_limit);
        _value[index] = static_cast<int16_t>(value);

        if ((info.type == Type::ControlChange14Bit) && (info.midi_index >= protocol::midi::CONTROL_CHANGE_14BIT_MAX_INDEX))
        {
            return {};
        }
    }
    break;

    case Type::SingleNoteFixedValBothDir:
    case Type::SingleNoteFixedValOneDir0OtherDir:
    case Type::TwoNoteFixedValBothDir:
    {
        value = info.repeated_value;

        if ((info.type == Type::SingleNoteFixedValOneDir0OtherDir) && (position == Position::Ccw))
        {
            value = 0;
        }
    }
    break;

    case Type::PresetChange:
    {
        result.system = signaling::SystemSignal{
            .system_event = (position == Position::Cw) ? signaling::SystemEvent::PresetChangeIncReq
                                                       : signaling::SystemEvent::PresetChangeDecReq,
        };
        return result;
    }

    case Type::BpmChange:
    {
        if (position == Position::Ccw)
        {
            if (!Bpm.increment(1))
            {
                return {};
            }
        }
        else
        {
            if (!Bpm.decrement(1))
            {
                return {};
            }
        }

        return {};
    }

    default:
        send = false;
        break;
    }

    if (!send)
    {
        return {};
    }

    result.osc.emplace();
    fill_osc_signal(*result.osc, info, value);

    result.midi.emplace();
    fill_midi_signal(*result.midi, info, value);

    return result;
}

std::optional<Mapper::Result> Mapper::last_result(size_t index) const
{
    if (index >= Collection::size())
    {
        return {};
    }

    const auto info = read_database_info(index, Position::Stopped);

    if (!has_refresh_value(info.type))
    {
        return {};
    }

    uint16_t value = 0;

    switch (info.type)
    {
    case Type::ProgramChange:
        value = MidiProgram.program(info.channel);
        break;

    case Type::ControlChange:
    case Type::PitchBend:
    case Type::Nrpn7Bit:
    case Type::Nrpn14Bit:
    case Type::ControlChange14Bit:
    case Type::SingleNoteVariableVal:
        value = static_cast<uint16_t>(_value[index]);
        break;

    default:
        return {};
    }

    if ((info.type == Type::ControlChange14Bit) && (info.midi_index >= protocol::midi::CONTROL_CHANGE_14BIT_MAX_INDEX))
    {
        return {};
    }

    Result result = {};
    result.osc.emplace();
    fill_osc_signal(*result.osc, info, value);
    result.midi.emplace();
    fill_midi_signal(*result.midi, info, value);
    return result;
}

void Mapper::reset(size_t index)
{
    if (index >= Collection::size())
    {
        return;
    }

    if (_database.read(database::Config::Section::Encoder::Mode, index) == static_cast<int32_t>(Type::PitchBend))
    {
        _value[index] = static_cast<int16_t>(midi::MIDI_PITCH_BEND_CENTER);
    }
    else
    {
        _value[index] = 0;
    }
}

void Mapper::set_value(size_t index, uint16_t value)
{
    if (index >= Collection::size())
    {
        return;
    }

    _value[index] = static_cast<int16_t>(value);
}

Mapper::DatabaseInfo Mapper::read_database_info(size_t index, Position position) const
{
    DatabaseInfo info    = {};
    info.type            = static_cast<Type>(_database.read(database::Config::Section::Encoder::Mode, index));
    info.inverted        = _database.read(database::Config::Section::Encoder::Invert, index);
    info.component_index = index;
    info.channel         = _database.read(database::Config::Section::Encoder::Channel, index);
    info.repeated_value  = _database.read(database::Config::Section::Encoder::RepeatedValue, index);
    info.lower_limit     = _database.read(database::Config::Section::Encoder::LowerLimit, index);
    info.upper_limit     = _database.read(database::Config::Section::Encoder::UpperLimit, index);
    info.midi_message    = INTERNAL_MSG_TO_MIDI_TYPE[static_cast<uint8_t>(info.type)];

    switch (info.type)
    {
    case Type::TwoNoteFixedValBothDir:
        info.midi_index = (position == Position::Ccw)
                              ? _database.read(database::Config::Section::Encoder::MidiId2, index)
                              : _database.read(database::Config::Section::Encoder::MidiId1, index);
        break;

    default:
        info.midi_index = _database.read(database::Config::Section::Encoder::MidiId1, index);
        break;
    }

    return info;
}

void Mapper::fill_midi_signal(signaling::MidiIoSignal& signal, const DatabaseInfo& info, uint16_t value) const
{
    signal.source          = signaling::IoEventSource::Encoder;
    signal.component_index = info.component_index;
    signal.channel         = info.channel;
    signal.index           = info.midi_index;
    signal.value           = value;
    signal.message         = info.midi_message;
}

void Mapper::fill_osc_signal(signaling::OscIoSignal& signal, const DatabaseInfo& info, uint16_t value) const
{
    signal.source          = signaling::IoEventSource::Encoder;
    signal.component_index = info.component_index;
    signal.int32_value     = static_cast<int32_t>(value);
    signal.direction       = signaling::SignalDirection::Out;
}

bool Mapper::has_refresh_value(Type type) const
{
    switch (type)
    {
    case Type::ProgramChange:
    case Type::ControlChange:
    case Type::PitchBend:
    case Type::Nrpn7Bit:
    case Type::Nrpn14Bit:
    case Type::ControlChange14Bit:
    case Type::SingleNoteVariableVal:
        return true;

    default:
        return false;
    }
}
