/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mapper.h"

#include "global/bpm.h"
#include "global/midi_program.h"
#include "util/incdec/inc_dec.h"
#include "util/conversion/conversion.h"

using namespace opendeck::io::switches;
using namespace opendeck::protocol;

namespace
{
    using ValueIncDecMidi7Bit = opendeck::util::IncDec<uint8_t, 0, opendeck::protocol::midi::MAX_VALUE_7BIT>;

    constexpr std::array<opendeck::protocol::midi::MessageType, static_cast<uint8_t>(MessageType::Count)> INTERNAL_MSG_TO_MIDI_TYPE = {
        opendeck::protocol::midi::MessageType::NoteOn,                      // Note
        opendeck::protocol::midi::MessageType::ProgramChange,               // ProgramChange
        opendeck::protocol::midi::MessageType::ControlChange,               // ControlChange
        opendeck::protocol::midi::MessageType::ControlChange,               // ControlChangeReset
        opendeck::protocol::midi::MessageType::MmcStop,                     // MmcStop
        opendeck::protocol::midi::MessageType::MmcPlay,                     // MmcPlay
        opendeck::protocol::midi::MessageType::MmcRecordStart,              // MmcRecord
        opendeck::protocol::midi::MessageType::MmcPause,                    // MmcPause
        opendeck::protocol::midi::MessageType::SysRealTimeClock,            // RealTimeClock
        opendeck::protocol::midi::MessageType::SysRealTimeStart,            // RealTimeStart
        opendeck::protocol::midi::MessageType::SysRealTimeContinue,         // RealTimeContinue
        opendeck::protocol::midi::MessageType::SysRealTimeStop,             // RealTimeStop
        opendeck::protocol::midi::MessageType::SysRealTimeActiveSensing,    // RealTimeActiveSensing
        opendeck::protocol::midi::MessageType::SysRealTimeSystemReset,      // RealTimeSystemReset
        opendeck::protocol::midi::MessageType::ProgramChange,               // ProgramChangeInc
        opendeck::protocol::midi::MessageType::ProgramChange,               // ProgramChangeDec
        opendeck::protocol::midi::MessageType::Invalid,                     // None
        opendeck::protocol::midi::MessageType::Invalid,                     // PresetChange
        opendeck::protocol::midi::MessageType::NoteOn,                      // MultiValIncResetNote
        opendeck::protocol::midi::MessageType::NoteOn,                      // MultiValIncDecNote
        opendeck::protocol::midi::MessageType::ControlChange,               // MultiValIncResetCc
        opendeck::protocol::midi::MessageType::ControlChange,               // MultiValIncDecCc
        opendeck::protocol::midi::MessageType::NoteOn,                      // NoteOffOnly
        opendeck::protocol::midi::MessageType::ControlChange,               // ControlChange0Only
        opendeck::protocol::midi::MessageType::Invalid,                     // Reserved
        opendeck::protocol::midi::MessageType::Invalid,                     // ProgramChangeOffsetInc
        opendeck::protocol::midi::MessageType::Invalid,                     // ProgramChangeOffsetDec
        opendeck::protocol::midi::MessageType::Invalid,                     // BpmInc
        opendeck::protocol::midi::MessageType::Invalid,                     // BpmDec
        opendeck::protocol::midi::MessageType::MmcPlay,                     // MmcPlayStop
    };

}    // namespace

std::optional<Mapper::Result> Mapper::result(size_t index, bool reading)
{
    if (index >= Collection::size())
    {
        return {};
    }

    if (_pressed_state[index] == reading)
    {
        return {};
    }

    _pressed_state[index] = reading;

    return build_result(index, reading, read_database_info(index), true, false);
}

std::optional<Mapper::Result> Mapper::refresh_result(size_t index)
{
    if (index >= Collection::size())
    {
        return {};
    }

    return build_result(index, _pressed_state[index], read_database_info(index), false, true);
}

void Mapper::reset(size_t index)
{
    if (index >= Collection::size())
    {
        return;
    }

    _pressed_state[index]       = false;
    _osc_latching_state[index]  = false;
    _midi_latching_state[index] = false;
    _inc_dec_value[index]       = 0;
}

Mapper::DatabaseInfo Mapper::read_database_info(size_t index) const
{
    DatabaseInfo info    = {};
    info.type            = static_cast<Type>(_database.read(database::Config::Section::Switch::Type, index));
    info.message_type    = static_cast<MessageType>(_database.read(database::Config::Section::Switch::MessageType, index));
    info.component_index = index;
    info.channel         = _database.read(database::Config::Section::Switch::Channel, index);
    info.midi_index      = _database.read(database::Config::Section::Switch::MidiId, index);
    info.midi_value      = _database.read(database::Config::Section::Switch::Value, index);
    info.midi_message    = INTERNAL_MSG_TO_MIDI_TYPE[static_cast<uint8_t>(info.message_type)];

    return info;
}

Type Mapper::message_to_type(MessageType message_type, Type configured_type)
{
    switch (message_type)
    {
    case MessageType::ProgramChange:
    case MessageType::ProgramChangeInc:
    case MessageType::ProgramChangeDec:
    case MessageType::MmcPlay:
    case MessageType::MmcStop:
    case MessageType::MmcPause:
    case MessageType::ControlChange:
    case MessageType::RealTimeClock:
    case MessageType::RealTimeStart:
    case MessageType::RealTimeContinue:
    case MessageType::RealTimeStop:
    case MessageType::RealTimeActiveSensing:
    case MessageType::RealTimeSystemReset:
    case MessageType::MultiValIncResetNote:
    case MessageType::MultiValIncDecNote:
    case MessageType::MultiValIncResetCc:
    case MessageType::MultiValIncDecCc:
    case MessageType::PresetChange:
    case MessageType::ProgramChangeOffsetInc:
    case MessageType::ProgramChangeOffsetDec:
    case MessageType::NoteOffOnly:
    case MessageType::ControlChange0Only:
    case MessageType::BpmInc:
    case MessageType::BpmDec:
        return Type::Momentary;

    case MessageType::MmcRecord:
    case MessageType::MmcPlayStop:
        return Type::Latching;

    default:
        return configured_type;
    }
}

std::optional<Mapper::Result> Mapper::build_result(size_t              index,
                                                   bool                reading,
                                                   const DatabaseInfo& info,
                                                   bool                mutate,
                                                   bool                refresh)
{
    if (index >= Collection::size())
    {
        return {};
    }

    Result result = {};

    bool osc_send  = true;
    bool osc_state = reading;

    if (info.type == Type::Latching)
    {
        if (refresh)
        {
            osc_state = _osc_latching_state[index];
        }
        else if (reading)
        {
            if (mutate)
            {
                _osc_latching_state[index] = !_osc_latching_state[index];
            }

            osc_state = _osc_latching_state[index];
        }
        else
        {
            osc_send = false;
        }
    }

    if (osc_send)
    {
        result.osc.emplace();
        fill_osc_signal(result, index, osc_state);
    }

    bool midi_send  = true;
    bool midi_state = reading;

    if (message_to_type(info.message_type, info.type) == Type::Latching)
    {
        if (refresh)
        {
            midi_state = _midi_latching_state[index];
        }
        else if (reading)
        {
            if (mutate)
            {
                _midi_latching_state[index] = !_midi_latching_state[index];
            }

            midi_state = _midi_latching_state[index];
        }
        else
        {
            midi_send = false;
        }
    }

    if (midi_send)
    {
        bool                    send   = true;
        signaling::MidiIoSignal signal = {};
        fill_midi_signal(signal, info);

        if (midi_state)
        {
            switch (info.message_type)
            {
            case MessageType::Note:
            case MessageType::ControlChange:
            case MessageType::ControlChangeReset:
            case MessageType::RealTimeClock:
            case MessageType::RealTimeStart:
            case MessageType::RealTimeContinue:
            case MessageType::RealTimeStop:
            case MessageType::RealTimeActiveSensing:
            case MessageType::RealTimeSystemReset:
            case MessageType::MmcPlay:
            case MessageType::MmcStop:
            case MessageType::MmcPause:
            case MessageType::MmcRecord:
            case MessageType::MmcPlayStop:
                break;

            case MessageType::ProgramChange:
            {
                signal.value = 0;
                signal.index += MidiProgram.offset();
                signal.index &= protocol::midi::MAX_VALUE_7BIT;
            }
            break;

            case MessageType::ProgramChangeInc:
            {
                signal.value = 0;

                if (mutate && !MidiProgram.increment_program(signal.channel, 1))
                {
                    send = false;
                }

                signal.index = MidiProgram.program(signal.channel);
            }
            break;

            case MessageType::ProgramChangeDec:
            {
                signal.value = 0;

                if (mutate && !MidiProgram.decrement_program(signal.channel, 1))
                {
                    send = false;
                }

                signal.index = MidiProgram.program(signal.channel);
            }
            break;

            case MessageType::MultiValIncResetNote:
            {
                const auto current_value = _inc_dec_value[index];
                const auto new_value     = mutate
                                               ? ValueIncDecMidi7Bit::increment(current_value,
                                                                                signal.value,
                                                                                ValueIncDecMidi7Bit::Type::Overflow)
                                               : current_value;

                if (mutate && (new_value == current_value))
                {
                    send = false;
                }
                else
                {
                    signal.message = new_value == 0 ? protocol::midi::MessageType::NoteOff
                                                    : protocol::midi::MessageType::NoteOn;

                    if (mutate)
                    {
                        _inc_dec_value[index] = new_value;
                    }

                    signal.value = new_value;
                }
            }
            break;

            case MessageType::MultiValIncDecNote:
            {
                const auto current_value = _inc_dec_value[index];
                const auto new_value     = mutate
                                               ? ValueIncDecMidi7Bit::increment(current_value,
                                                                                signal.value,
                                                                                ValueIncDecMidi7Bit::Type::Edge)
                                               : current_value;

                if (mutate && (new_value == current_value))
                {
                    send = false;
                }
                else
                {
                    signal.message = new_value == 0 ? protocol::midi::MessageType::NoteOff
                                                    : protocol::midi::MessageType::NoteOn;

                    if (mutate)
                    {
                        _inc_dec_value[index] = new_value;
                    }

                    signal.value = new_value;
                }
            }
            break;

            case MessageType::MultiValIncResetCc:
            {
                const auto current_value = _inc_dec_value[index];
                const auto new_value     = mutate
                                               ? ValueIncDecMidi7Bit::increment(current_value,
                                                                                signal.value,
                                                                                ValueIncDecMidi7Bit::Type::Overflow)
                                               : current_value;

                if (mutate && (new_value == current_value))
                {
                    send = false;
                }
                else
                {
                    if (mutate)
                    {
                        _inc_dec_value[index] = new_value;
                    }

                    signal.value = new_value;
                }
            }
            break;

            case MessageType::MultiValIncDecCc:
            {
                const auto current_value = _inc_dec_value[index];
                const auto new_value     = mutate
                                               ? ValueIncDecMidi7Bit::increment(current_value,
                                                                                signal.value,
                                                                                ValueIncDecMidi7Bit::Type::Edge)
                                               : current_value;

                if (mutate && (new_value == current_value))
                {
                    send = false;
                }
                else
                {
                    if (mutate)
                    {
                        _inc_dec_value[index] = new_value;
                    }

                    signal.value = new_value;
                }
            }
            break;

            case MessageType::NoteOffOnly:
            {
                signal.value   = 0;
                signal.message = protocol::midi::MessageType::NoteOff;
            }
            break;

            case MessageType::ControlChange0Only:
            {
                signal.value = 0;
            }
            break;

            case MessageType::ProgramChangeOffsetInc:
            {
                if (mutate)
                {
                    MidiProgram.increment_offset(signal.value);
                }
            }
            break;

            case MessageType::ProgramChangeOffsetDec:
            {
                if (mutate)
                {
                    MidiProgram.decrement_offset(signal.value);
                }
            }
            break;

            case MessageType::PresetChange:
            {
                if (mutate)
                {
                    result.system.emplace();
                    result.system->system_event = signaling::SystemEvent::PresetChangeDirectReq;
                    result.system->value        = signal.index;
                }

                send = false;
            }
            break;

            case MessageType::BpmInc:
            {
                signal.value = 0;

                if (mutate && !Bpm.increment(1))
                {
                    send = false;
                }

                signal.index = Bpm.value();
            }
            break;

            case MessageType::BpmDec:
            {
                signal.value = 0;

                if (mutate && !Bpm.decrement(1))
                {
                    send = false;
                }

                signal.index = Bpm.value();
            }
            break;

            case MessageType::None:
            default:
            {
                send = false;
            }
            break;
            }
        }
        else
        {
            switch (info.message_type)
            {
            case MessageType::Note:
            {
                signal.value   = 0;
                signal.message = protocol::midi::MessageType::NoteOff;
            }
            break;

            case MessageType::ControlChangeReset:
            {
                signal.value = 0;
            }
            break;

            case MessageType::MmcRecord:
            {
                signal.message = protocol::midi::MessageType::MmcRecordStop;
            }
            break;

            case MessageType::MmcPlayStop:
            {
                signal.message = protocol::midi::MessageType::MmcStop;
            }
            break;

            default:
            {
                send = false;
            }
            break;
            }
        }

        if (send)
        {
            signal.source = signaling::IoEventSource::Switch;
            result.midi   = signal;
        }
    }

    if (!result.osc.has_value() && !result.midi.has_value() && !result.system.has_value())
    {
        return {};
    }

    return result;
}

void Mapper::fill_midi_signal(signaling::MidiIoSignal& signal, const DatabaseInfo& info) const
{
    signal.component_index = info.component_index;
    signal.channel         = info.channel;
    signal.index           = info.midi_index;
    signal.value           = info.midi_value;
    signal.message         = info.midi_message;
}

void Mapper::fill_osc_signal(Result& result, size_t index, bool state) const
{
    result.osc->source          = signaling::IoEventSource::Switch;
    result.osc->component_index = index;
    result.osc->int32_value     = static_cast<int32_t>(state ? 1 : 0);
    result.osc->float_value     = state ? 1.0F : 0.0F;
    result.osc->direction       = signaling::SignalDirection::Out;
}
