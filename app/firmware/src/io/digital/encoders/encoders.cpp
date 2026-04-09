/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ENCODERS

#include "encoders.h"
#include "util/conversion/conversion.h"
#include "util/configurable/configurable.h"
#include "global/bpm.h"

#include "zlibs/utils/misc/bit.h"
#include "zlibs/utils/misc/numeric.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

using namespace io::encoders;
using namespace protocol;

namespace
{
    LOG_MODULE_REGISTER(encoders, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

Encoders::Encoders(Hwa&      hwa,
                   Filter&   filter,
                   Database& database)
    : _hwa(hwa)
    , _filter(filter)
    , _database(database)
{
    messaging::subscribe<messaging::UmpSignal>(
        [this](const messaging::UmpSignal& event)
        {
            if (is_frozen())
            {
                return;
            }

            if (event.direction != messaging::MidiDirection::In)
            {
                return;
            }

            const auto message = midi::decode_message(event.packet);

            const uint8_t global_channel = _database.read(database::Config::Section::Global::MidiSettings, midi::Setting::GlobalChannel);
            const uint8_t channel        = _database.read(database::Config::Section::Global::MidiSettings,
                                                          midi::Setting::UseGlobalChannel)
                                               ? global_channel
                                               : message.channel;

            const bool use_omni = channel == midi::OMNI_CHANNEL ? true : false;

            switch (message.type)
            {
            case midi::MessageType::ControlChange:
            {
                for (size_t i = 0; i < Collection::size(); i++)
                {
                    if (!_database.read(database::Config::Section::Encoder::RemoteSync, i))
                    {
                        continue;
                    }

                    if (_database.read(database::Config::Section::Encoder::Mode, i) != static_cast<int32_t>(Type::ControlChange))
                    {
                        continue;
                    }

                    if (!use_omni)
                    {
                        if (_database.read(database::Config::Section::Encoder::Channel, i) != channel)
                        {
                            continue;
                        }
                    }

                    if (_database.read(database::Config::Section::Encoder::MidiId1, i) != message.data1)
                    {
                        continue;
                    }

                    set_value(i, message.data2);
                }
            }
            break;

            default:
                break;
            }
        });

    ConfigHandler.register_config(
        sys::Config::Block::Encoders,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sys_config_get(static_cast<sys::Config::Section::Encoder>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sys_config_set(static_cast<sys::Config::Section::Encoder>(section), index, value);
        });
}

Encoders::~Encoders()
{
    shutdown();
}

void Encoders::deinit()
{
    shutdown();
}

void Encoders::shutdown()
{
}

bool Encoders::init()
{
    for (size_t i = 0; i < Collection::size(); i++)
    {
        reset(i);
    }

    return true;
}

size_t Encoders::refreshable_components() const
{
    return Collection::size();
}

void Encoders::force_refresh(size_t start_index, size_t count)
{
    const auto total = Collection::size();

    if (start_index >= total)
    {
        return;
    }

    const auto end = std::min(start_index + count, total);

    for (size_t i = start_index; i < end; i++)
    {
        if (!_database.read(database::Config::Section::Encoder::Enable, i))
        {
            continue;
        }

        Descriptor descriptor;
        fill_descriptor(i, Position::Stopped, descriptor);

        switch (descriptor.type)
        {
        case Type::ProgramChange:
        {
            descriptor.signal.value = MidiProgram.program(descriptor.signal.channel);
        }
        break;

        case Type::ControlChange:
        case Type::PitchBend:
        case Type::Nrpn7Bit:
        case Type::Nrpn14Bit:
        case Type::ControlChange14Bit:
        case Type::SingleNoteVariableVal:
        {
            descriptor.signal.value = _value[i];
        }
        break;

        default:
            continue;
        }

        messaging::publish(descriptor.signal);
    }
}

void Encoders::process_state_changes()
{
    if (is_frozen())
    {
        return;
    }

    for (size_t i = 0; i < Collection::size(); i++)
    {
        if (!_database.read(database::Config::Section::Encoder::Enable, i))
        {
            continue;
        }

        const auto state = _hwa.state(i);

        if (!state.has_value())
        {
            continue;
        }

        process_reading(i, state.value(), k_uptime_get_32());
    }
}

void Encoders::process_reading(size_t index, uint8_t pair_value, uint32_t sample_time)
{
    auto position = read(index, pair_value);

    if (_filter.is_filtered(index, position, position, sample_time))
    {
        if (position != Position::Stopped)
        {
            if (_database.read(database::Config::Section::Encoder::Invert, index))
            {
                if (position == Position::Ccw)
                {
                    position = Position::Cw;
                }
                else
                {
                    position = Position::Ccw;
                }
            }

            uint8_t enc_acceleration = _database.read(database::Config::Section::Encoder::Acceleration, index);

            if (enc_acceleration)
            {
                // when time difference between two movements is smaller than ENCODERS_SPEED_TIMEOUT,
                // start accelerating
                if ((sample_time - _filter.last_movement_time(index)) < ENCODERS_SPEED_TIMEOUT_MS)
                {
                    _encoder_speed[index] = zlibs::utils::misc::constrain(static_cast<uint8_t>(_encoder_speed[index] + ENCODER_SPEED_CHANGE[enc_acceleration]),
                                                                          static_cast<uint8_t>(0),
                                                                          ENCODER_ACCELERATION_STEP_INC[enc_acceleration]);
                }
                else
                {
                    _encoder_speed[index] = 0;
                }
            }

            Descriptor descriptor;
            fill_descriptor(index, position, descriptor);

            send_message(index, position, descriptor);
        }
    }
}

void Encoders::send_message(size_t index, Position position, Descriptor& descriptor, bool ignore_freeze)
{
    if (is_frozen() && !ignore_freeze)
    {
        return;
    }

    bool    send  = true;
    uint8_t steps = (_encoder_speed[index] > 0) ? _encoder_speed[index] : 1;

    switch (descriptor.type)
    {
    case Type::ControlChange7fh01h:
    {
        descriptor.signal.value = VAL_CONTROL_CHANGE_7FH01H[static_cast<uint8_t>(position)];
    }
    break;

    case Type::ControlChange3fh41h:
    {
        descriptor.signal.value = VAL_CONTROL_CHANGE_3FH41H[static_cast<uint8_t>(position)];
    }
    break;

    case Type::ControlChange41h01h:
    {
        descriptor.signal.value = VAL_CONTROL_CHANGE_41H01H[static_cast<uint8_t>(position)];
    }
    break;

    case Type::ProgramChange:
    {
        if (position == Position::Ccw)
        {
            if (!MidiProgram.increment_program(descriptor.signal.channel, 1))
            {
                return;
            }
        }
        else
        {
            if (!MidiProgram.decrement_program(descriptor.signal.channel, 1))
            {
                return;
            }
        }

        descriptor.signal.value = MidiProgram.program(descriptor.signal.channel);
    }
    break;

    case Type::ControlChange:
    case Type::PitchBend:
    case Type::Nrpn7Bit:
    case Type::Nrpn14Bit:
    case Type::ControlChange14Bit:
    case Type::SingleNoteVariableVal:
    {
        const bool use_14bit =
            ((descriptor.type == Type::PitchBend) || (descriptor.type == Type::Nrpn14Bit) || (descriptor.type == Type::ControlChange14Bit));

        if (use_14bit && (steps > 1))
        {
            steps <<= 2;
        }

        if (position == Position::Ccw)
        {
            // ValueIncDecMidi7Bit is used, but any type can be used when decrementing since the limit is 0 for all of them
            _value[index] = ValueIncDecMidi7Bit::decrement(_value[index],
                                                           steps,
                                                           ValueIncDecMidi7Bit::Type::Edge);
        }
        else
        {
            switch (descriptor.type)
            {
            case Type::ControlChange:
            case Type::Nrpn7Bit:
            case Type::SingleNoteVariableVal:
            {
                _value[index] = ValueIncDecMidi7Bit::increment(_value[index],
                                                               steps,
                                                               ValueIncDecMidi7Bit::Type::Edge);
            }
            break;

            case Type::PitchBend:
            case Type::Nrpn14Bit:
            case Type::ControlChange14Bit:
            {
                const auto incremented_value = ValueIncDecMidi14Bit::increment(static_cast<uint16_t>(_value[index]),
                                                                               steps,
                                                                               ValueIncDecMidi14Bit::Type::Edge);

                _value[index] = static_cast<int16_t>(incremented_value);
            }
            break;

            default:
                break;
            }
        }

        const auto constrained_value = zlibs::utils::misc::constrain(static_cast<uint16_t>(_value[index]),
                                                                     static_cast<uint16_t>(_database.read(database::Config::Section::Encoder::LowerLimit, index)),
                                                                     static_cast<uint16_t>(_database.read(database::Config::Section::Encoder::UpperLimit, index)));

        _value[index] = static_cast<int16_t>(constrained_value);

        if (descriptor.type == Type::ControlChange14Bit)
        {
            if (descriptor.signal.index >= protocol::midi::CONTROL_CHANGE_14BIT_MAX_INDEX)
            {
                return;
            }
        }

        descriptor.signal.value = _value[index];
    }
    break;

    case Type::SingleNoteFixedValBothDir:
    case Type::SingleNoteFixedValOneDir0OtherDir:
    case Type::TwoNoteFixedValBothDir:
    {
        descriptor.signal.value = _database.read(database::Config::Section::Encoder::RepeatedValue, index);

        if (descriptor.type == Type::SingleNoteFixedValOneDir0OtherDir)
        {
            if (position == Position::Ccw)
            {
                descriptor.signal.value = 0;
            }
        }
    }
    break;

    case Type::PresetChange:
    {
        messaging::SystemSignal signal = {};
        signal.system_message          = (position == Position::Cw)
                                             ? messaging::SystemMessage::PresetChangeIncReq
                                             : messaging::SystemMessage::PresetChangeDecReq;
        messaging::publish(signal);
        return;
    }
    break;

    case Type::BpmChange:
    {
        if (position == Position::Ccw)
        {
            if (!Bpm.increment(1))
            {
                return;
            }
        }
        else
        {
            if (!Bpm.decrement(1))
            {
                return;
            }
        }
        return;
    }
    break;

    default:
    {
        send = false;
    }
    break;
    }

    if (send)
    {
        messaging::publish(descriptor.signal);
    }
}

void Encoders::reset(size_t index)
{
    if (_database.read(database::Config::Section::Encoder::Mode, index) == static_cast<int32_t>(Type::PitchBend))
    {
        _value[index] = static_cast<int16_t>(midi::MIDI_PITCH_BEND_CENTER);
    }
    else
    {
        _value[index] = 0;
    }

    _filter.reset(index);
    _encoder_speed[index]  = 0;
    _encoder_data[index]   = 0;
    _encoder_pulses[index] = 0;
}

void Encoders::set_value(size_t index, uint16_t value)
{
    _value[index] = static_cast<int16_t>(value);
}

Position Encoders::read(size_t index, uint8_t pair_state)
{
    static constexpr uint8_t ENCODER_STATE_MASK      = 0x03;
    static constexpr uint8_t ENCODER_DATA_VALID_BIT  = 7;
    static constexpr uint8_t ENCODER_DATA_VALID_MASK = 0x80;
    static constexpr uint8_t ENCODER_LOOKUP_MASK     = 0x0F;

    auto position = Position::Stopped;
    pair_state &= ENCODER_STATE_MASK;

    // add new data

    bool process = true;

    // only process the data from encoder if there is a previous reading stored
    if (!zlibs::utils::misc::bit_read(_encoder_data[index], ENCODER_DATA_VALID_BIT))
    {
        process = false;
    }

    _encoder_data[index] <<= 2;
    _encoder_data[index] |= pair_state;
    _encoder_data[index] |= ENCODER_DATA_VALID_MASK;

    if (!process)
    {
        return position;
    }

    _encoder_pulses[index] = static_cast<int8_t>(_encoder_pulses[index] + ENCODER_LOOK_UP_TABLE[_encoder_data[index] & ENCODER_LOOKUP_MASK]);

    if (abs(_encoder_pulses[index]) >= static_cast<int32_t>(_database.read(database::Config::Section::Encoder::PulsesPerStep, index)))
    {
        position = (_encoder_pulses[index] > 0) ? Position::Ccw : Position::Cw;
        // reset count
        _encoder_pulses[index] = 0;
    }

    return position;
}

void Encoders::fill_descriptor(size_t index, Position position, Descriptor& descriptor)
{
    descriptor.type = static_cast<Type>(_database.read(database::Config::Section::Encoder::Mode, index));

    switch (descriptor.type)
    {
    case Type::TwoNoteFixedValBothDir:
    {
        if (position == Position::Ccw)
        {
            descriptor.signal.index = _database.read(database::Config::Section::Encoder::MidiId2, index);
        }
        else
        {
            descriptor.signal.index = _database.read(database::Config::Section::Encoder::MidiId1, index);
        }
    }
    break;

    default:
    {
        descriptor.signal.index = _database.read(database::Config::Section::Encoder::MidiId1, index);
    }
    break;
    }

    descriptor.signal.source          = messaging::MidiSource::Encoder;
    descriptor.signal.component_index = index;
    descriptor.signal.channel         = _database.read(database::Config::Section::Encoder::Channel, index);
    descriptor.signal.message         = INTERNAL_MSG_TO_MIDI_TYPE[static_cast<uint8_t>(descriptor.type)];
}

std::optional<uint8_t> Encoders::sys_config_get(sys::Config::Section::Encoder section, size_t index, uint16_t& value)
{
    uint32_t read_value = 0;

    auto result = _database.read(util::Conversion::sys_2_db_section(section), index, read_value)
                      ? sys::Config::Status::Ack
                      : sys::Config::Status::ErrorRead;

    if (result == sys::Config::Status::Ack)
    {
        if (section == sys::Config::Section::Encoder::Reserved1)
        {
            return sys::Config::Status::ErrorNotSupported;
        }
    }

    value = read_value;

    return result;
}

std::optional<uint8_t> Encoders::sys_config_set(sys::Config::Section::Encoder section, size_t index, uint16_t value)
{
    switch (section)
    {
    case sys::Config::Section::Encoder::Reserved1:
        return sys::Config::Status::ErrorNotSupported;

    default:
        break;
    }

    auto result = _database.update(util::Conversion::sys_2_db_section(section), index, value)
                      ? sys::Config::Status::Ack
                      : sys::Config::Status::ErrorWrite;

    if (result == sys::Config::Status::Ack)
    {
        reset(index);
    }

    return result;
}

#endif
