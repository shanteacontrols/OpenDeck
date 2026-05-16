/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ENCODERS

#include "firmware/src/io/digital/encoders/instance/impl/encoders.h"
#include "firmware/src/util/conversion/conversion.h"
#include "firmware/src/util/configurable/configurable.h"
#include "firmware/src/global/bpm.h"

#include "zlibs/utils/misc/bit.h"
#include "zlibs/utils/misc/numeric.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

using namespace opendeck;
using namespace opendeck::io::encoders;
using namespace opendeck::protocol;

namespace
{
    LOG_MODULE_REGISTER(encoders, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

Encoders::Encoders(Hwa&      hwa,
                   Filter&   filter,
                   Database& database,
                   Mapper&   mapper)
    : _hwa(hwa)
    , _filter(filter)
    , _database(database)
    , _mapper(mapper)
{
    signaling::subscribe<signaling::UmpSignal>(
        [this](const signaling::UmpSignal& event)
        {
            if (is_frozen())
            {
                return;
            }

            if (event.direction != signaling::SignalDirection::In)
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

                    _mapper.set_value(i, message.data2);
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

        const auto result = _mapper.last_result(i);

        if (!result.has_value())
        {
            continue;
        }

        if (result->osc.has_value())
        {
            signaling::publish(result->osc.value());
        }

        if (result->midi.has_value())
        {
            signaling::publish(result->midi.value());
        }

        if (result->system.has_value())
        {
            signaling::publish(result->system.value());
        }
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
    Position position = Position::Stopped;

    if (_filter.is_filtered(index,
                            pair_value,
                            position,
                            sample_time))
    {
        if (position != Position::Stopped)
        {
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

            const auto result = _mapper.result(index, position, (_encoder_speed[index] > 0) ? _encoder_speed[index] : 1);

            if (!result.has_value())
            {
                return;
            }

            if (result->osc.has_value())
            {
                signaling::publish(result->osc.value());
            }

            if (result->midi.has_value())
            {
                signaling::publish(result->midi.value());
            }

            if (result->system.has_value())
            {
                signaling::publish(result->system.value());
            }
        }
    }
}

void Encoders::reset(size_t index)
{
    _mapper.reset(index);
    _filter.reset(index);
    _encoder_speed[index] = 0;
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
