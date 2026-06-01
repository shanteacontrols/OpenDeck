/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/io/outputs/instance/impl/mapper.h"
#include "firmware/src/global/midi_program.h"

#include "zlibs/utils/misc/numeric.h"

using namespace opendeck;
using namespace opendeck::io::outputs;
using namespace opendeck::firmware;

namespace
{
    namespace zmidi = zlibs::utils::midi;
    namespace zmisc = zlibs::utils::misc;

    constexpr uint8_t MIDI_VALUE_GROUP_SIZE = 16;

    constexpr protocol::midi::MessageType CONTROL_TYPE_TO_MIDI_MESSAGE[static_cast<uint8_t>(ControlType::Count)] = {
        protocol::midi::MessageType::NoteOn,           // MIDI_IN_NOTE_SINGLE_VAL,
        protocol::midi::MessageType::NoteOn,           // LOCAL_NOTE_SINGLE_VAL,
        protocol::midi::MessageType::ControlChange,    // MIDI_IN_CC_SINGLE_VAL,
        protocol::midi::MessageType::ControlChange,    // LOCAL_CC_SINGLE_VAL,
        protocol::midi::MessageType::ProgramChange,    // PC_SINGLE_VAL,
        protocol::midi::MessageType::ProgramChange,    // Preset,
        protocol::midi::MessageType::NoteOn,           // MIDI_IN_NOTE_MULTI_VAL,
        protocol::midi::MessageType::NoteOn,           // LOCAL_NOTE_MULTI_VAL,
        protocol::midi::MessageType::ControlChange,    // MIDI_IN_CC_MULTI_VAL,
        protocol::midi::MessageType::ControlChange,    // LOCAL_CC_MULTI_VAL,
        protocol::midi::MessageType::Invalid,          // Static
    };
}    // namespace

Mapper::Result Mapper::midi_result(const protocol::midi::Message& message, signaling::SignalDirection direction)
{
    Result result = {};

    const uint8_t global_channel     = _database.read(database::Config::Section::Global::MidiSettings, protocol::midi::Setting::GlobalChannel);
    const uint8_t use_global_channel = _database.read(database::Config::Section::Global::MidiSettings,
                                                      protocol::midi::Setting::UseGlobalChannel);

    auto is_control_type_matched = [](protocol::midi::MessageType midi_message, ControlType control_type)
    {
        return CONTROL_TYPE_TO_MIDI_MESSAGE[static_cast<uint8_t>(control_type)] == midi_message;
    };

    auto midi_message = message.type;

    if (midi_message == protocol::midi::MessageType::NoteOff)
    {
        midi_message = protocol::midi::MessageType::NoteOn;
    }

    for (size_t i = 0; i < Collection::size(); i++)
    {
        auto control_type = static_cast<ControlType>(_database.read(database::Config::Section::Outputs::ControlType, i));

        if (!is_control_type_matched(midi_message, control_type))
        {
            continue;
        }

        bool set_state     = false;
        bool set_pulse     = false;
        bool check_channel = true;

        if (direction != signaling::SignalDirection::In)
        {
            switch (control_type)
            {
            case ControlType::LocalNoteSingleVal:
            {
                set_state = midi_message == protocol::midi::MessageType::NoteOn;
            }
            break;

            case ControlType::LocalCcSingleVal:
            {
                set_state = midi_message == protocol::midi::MessageType::ControlChange;
            }
            break;

            case ControlType::PcSingleVal:
            {
                set_state = midi_message == protocol::midi::MessageType::ProgramChange;
            }
            break;

            case ControlType::Preset:
            {
                check_channel = false;
            }
            break;

            case ControlType::LocalNoteMultiVal:
            {
                set_state = midi_message == protocol::midi::MessageType::NoteOn;
                set_pulse = set_state;
            }
            break;

            case ControlType::LocalCcMultiVal:
            {
                set_state = midi_message == protocol::midi::MessageType::ControlChange;
                set_pulse = set_state;
            }
            break;

            default:
                break;
            }
        }
        else
        {
            switch (control_type)
            {
            case ControlType::MidiInNoteSingleVal:
            {
                set_state = midi_message == protocol::midi::MessageType::NoteOn;
            }
            break;

            case ControlType::MidiInCcSingleVal:
            {
                set_state = midi_message == protocol::midi::MessageType::ControlChange;
            }
            break;

            case ControlType::PcSingleVal:
            {
                set_state = midi_message == protocol::midi::MessageType::ProgramChange;
            }
            break;

            case ControlType::MidiInNoteMultiVal:
            {
                set_state = midi_message == protocol::midi::MessageType::NoteOn;
                set_pulse = set_state;
            }
            break;

            case ControlType::MidiInCcMultiVal:
            {
                set_state = midi_message == protocol::midi::MessageType::ControlChange;
                set_pulse = set_state;
            }
            break;

            default:
                break;
            }
        }

        const auto db_channel = _database.read(database::Config::Section::Outputs::Channel, i);
        const bool use_omni   = (use_global_channel && (global_channel == protocol::midi::OMNI_CHANNEL)) || (db_channel == protocol::midi::OMNI_CHANNEL);

        if (check_channel && !use_omni)
        {
            const auto check_channel_value = use_global_channel ? global_channel : db_channel;

            if (check_channel_value != message.channel)
            {
                continue;
            }
        }

        if (!set_state && !set_pulse)
        {
            continue;
        }

        uint8_t activation_id = _database.read(database::Config::Section::Outputs::ActivationId, i);

        if (midi_message == protocol::midi::MessageType::ProgramChange)
        {
            if (_database.read(database::Config::Section::Outputs::Global, Setting::UseMidiProgramOffset))
            {
                activation_id += MidiProgram.offset();
                activation_id &= zmidi::MIDI_DATA_BYTE_MASK;
            }
        }

        const bool activation_id_matches = activation_id == message.data1;

        if (set_pulse && activation_id_matches)
        {
            result[i].pulse_speed = midi_value_to_pulse_speed(message.data2);
        }

        if (set_state)
        {
            if (activation_id_matches)
            {
                if (midi_message == protocol::midi::MessageType::ProgramChange)
                {
                    result[i].level_command = Action::LevelCommand::Set;
                    result[i].level         = OUTPUT_LEVEL_MAX;
                }
                else if (set_pulse)
                {
                    result[i].level_command = Action::LevelCommand::Set;
                    result[i].level         = midi_value_to_level(message.data2);
                }
                else
                {
                    result[i].level_command = Action::LevelCommand::Set;
                    result[i].level         = (_database.read(database::Config::Section::Outputs::ActivationValue, i) == message.data2) ? OUTPUT_LEVEL_MAX : OUTPUT_LEVEL_MIN;
                }
            }
            else if (midi_message == protocol::midi::MessageType::ProgramChange)
            {
                result[i].level_command = Action::LevelCommand::Set;
                result[i].level         = OUTPUT_LEVEL_MIN;
            }
        }
        else if (set_pulse && activation_id_matches)
        {
            result[i].level_command = Action::LevelCommand::Write;
        }
    }

    return result;
}

Mapper::Result Mapper::preset_result(uint8_t preset)
{
    Result result = {};

    for (size_t i = 0; i < Collection::size(); i++)
    {
        auto control_type = static_cast<ControlType>(_database.read(database::Config::Section::Outputs::ControlType, i));

        if (control_type != ControlType::Preset)
        {
            continue;
        }

        uint8_t activation_id = _database.read(database::Config::Section::Outputs::ActivationId, i);

        if (_database.read(database::Config::Section::Outputs::Global, Setting::UseMidiProgramOffset))
        {
            activation_id += MidiProgram.offset();
            activation_id &= zmidi::MIDI_DATA_BYTE_MASK;
        }

        result[i].level_command = Action::LevelCommand::Set;
        result[i].level         = activation_id == preset ? OUTPUT_LEVEL_MAX : OUTPUT_LEVEL_MIN;
    }

    return result;
}

Mapper::Action Mapper::osc_result(int32_t value) const
{
    if (value <= OUTPUT_LEVEL_MIN)
    {
        return {
            .level_command = Action::LevelCommand::Set,
            .level         = OUTPUT_LEVEL_MIN,
        };
    }

    if (value >= OUTPUT_LEVEL_MAX)
    {
        return {
            .level_command = Action::LevelCommand::Set,
            .level         = OUTPUT_LEVEL_MAX,
        };
    }

    return {
        .level_command = Action::LevelCommand::Set,
        .level         = static_cast<uint8_t>(value),
    };
}

PulseSpeed Mapper::midi_value_to_pulse_speed(uint8_t value) const
{
    if (value < MIDI_VALUE_GROUP_SIZE)
    {
        return PulseSpeed::NoPulse;
    }

    return static_cast<PulseSpeed>(value % MIDI_VALUE_GROUP_SIZE / 4);
}

uint8_t Mapper::midi_value_to_level(uint8_t value) const
{
    return static_cast<uint8_t>(zmisc::map_range(static_cast<uint32_t>(value),
                                                 0U,
                                                 static_cast<uint32_t>(protocol::midi::MAX_VALUE_7BIT),
                                                 static_cast<uint32_t>(OUTPUT_LEVEL_MIN),
                                                 static_cast<uint32_t>(OUTPUT_LEVEL_MAX)));
}
