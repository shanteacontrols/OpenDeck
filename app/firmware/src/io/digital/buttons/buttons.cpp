/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_BUTTONS

#include "buttons.h"
#include "system/config.h"
#include "global/midi_program.h"
#include "global/bpm.h"
#include "util/conversion/conversion.h"
#include "util/configurable/configurable.h"

#include "zlibs/utils/misc/bit.h"

#include <zephyr/logging/log.h>

using namespace opendeck;
using namespace opendeck::io::buttons;
using namespace opendeck::protocol;

namespace zmisc = zlibs::utils::misc;

namespace
{
    LOG_MODULE_REGISTER(buttons, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

Buttons::Buttons(Hwa&      hwa,
                 Filter&   filter,
                 Database& database)
    : _hwa(hwa)
    , _filter(filter)
    , _database(database)
{
    signaling::subscribe<signaling::MidiSignal>(
        [this](const signaling::MidiSignal& event)
        {
            if (is_frozen())
            {
                return;
            }

            if (event.source == signaling::MidiSource::AnalogButton)
            {
                size_t     index = event.component_index + Collection::start_index(GroupAnalogInputs);
                Descriptor descriptor;
                fill_descriptor(index, descriptor);

                process_button(index, event.value, descriptor);
                return;
            }

            if (event.source == signaling::MidiSource::TouchscreenButton)
            {
                size_t     index = event.component_index + Collection::start_index(GroupTouchscreenComponents);
                Descriptor descriptor;
                fill_descriptor(index, descriptor);

                process_button(index, event.value, descriptor);
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
        Descriptor descriptor;
        fill_descriptor(i, descriptor);

        if (descriptor.type == Type::Latching)
        {
            send_message(i, latching_state(i), descriptor, true);
        }
        else
        {
            send_message(i, cached_state(i), descriptor, true);
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
        Descriptor descriptor;

        auto current_state = state(i);

        if (!current_state.has_value())
        {
            continue;
        }

        fill_descriptor(i, descriptor);

        bool filtered_state = current_state.value();

        if (!_filter.is_filtered(i, filtered_state))
        {
            continue;
        }

        process_button(i, filtered_state, descriptor);
    }
}

void Buttons::process_button(size_t index, bool reading, Descriptor& descriptor)
{
    // act on change of state only
    if (reading == cached_state(index))
    {
        return;
    }

    set_state(index, reading);

    // don't process MessageType::None type of message
    if (descriptor.message_type != MessageType::None)
    {
        bool send = true;

        if (descriptor.type == Type::Latching)
        {
            // act on press only
            if (reading)
            {
                if (latching_state(index))
                {
                    set_latching_state(index, false);
                    // overwrite before processing
                    reading = false;
                }
                else
                {
                    set_latching_state(index, true);
                    reading = true;
                }
            }
            else
            {
                send = false;
            }
        }

        if (send)
        {
            send_message(index, reading, descriptor);
        }
    }
}

void Buttons::send_message(size_t index, bool state, Descriptor& descriptor, bool ignore_freeze)
{
    if (is_frozen() && !ignore_freeze)
    {
        return;
    }

    bool send = true;

    if (state)
    {
        switch (descriptor.message_type)
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
            descriptor.signal.value = 0;
            descriptor.signal.index += MidiProgram.offset();
            descriptor.signal.index &= protocol::midi::MAX_VALUE_7BIT;
        }
        break;

        case MessageType::ProgramChangeInc:
        {
            descriptor.signal.value = 0;

            if (!MidiProgram.increment_program(descriptor.signal.channel, 1))
            {
                send = false;
            }

            descriptor.signal.index = MidiProgram.program(descriptor.signal.channel);
        }
        break;

        case MessageType::ProgramChangeDec:
        {
            descriptor.signal.value = 0;

            if (!MidiProgram.decrement_program(descriptor.signal.channel, 1))
            {
                send = false;
            }

            descriptor.signal.index = MidiProgram.program(descriptor.signal.channel);
        }
        break;

        case MessageType::MultiValIncResetNote:
        {
            auto new_value = ValueIncDecMidi7Bit::increment(_inc_dec_value[index],
                                                            descriptor.signal.value,
                                                            ValueIncDecMidi7Bit::Type::Overflow);

            if (new_value != _inc_dec_value[index])
            {
                if (!new_value)
                {
                    descriptor.signal.message = protocol::midi::MessageType::NoteOff;
                }
                else
                {
                    descriptor.signal.message = protocol::midi::MessageType::NoteOn;
                }

                _inc_dec_value[index]   = new_value;
                descriptor.signal.value = new_value;
            }
            else
            {
                send = false;
            }
        }
        break;

        case MessageType::MultiValIncDecNote:
        {
            auto new_value = ValueIncDecMidi7Bit::increment(_inc_dec_value[index],
                                                            descriptor.signal.value,
                                                            ValueIncDecMidi7Bit::Type::Edge);

            if (new_value != _inc_dec_value[index])
            {
                if (!new_value)
                {
                    descriptor.signal.message = protocol::midi::MessageType::NoteOff;
                }
                else
                {
                    descriptor.signal.message = protocol::midi::MessageType::NoteOn;
                }

                _inc_dec_value[index]   = new_value;
                descriptor.signal.value = new_value;
            }
            else
            {
                send = false;
            }
        }
        break;

        case MessageType::MultiValIncResetCc:
        {
            auto new_value = ValueIncDecMidi7Bit::increment(_inc_dec_value[index],
                                                            descriptor.signal.value,
                                                            ValueIncDecMidi7Bit::Type::Overflow);

            if (new_value != _inc_dec_value[index])
            {
                _inc_dec_value[index]   = new_value;
                descriptor.signal.value = new_value;
            }
            else
            {
                send = false;
            }
        }
        break;

        case MessageType::MultiValIncDecCc:
        {
            auto new_value = ValueIncDecMidi7Bit::increment(_inc_dec_value[index],
                                                            descriptor.signal.value,
                                                            ValueIncDecMidi7Bit::Type::Edge);

            if (new_value != _inc_dec_value[index])
            {
                _inc_dec_value[index]   = new_value;
                descriptor.signal.value = new_value;
            }
            else
            {
                send = false;
            }
        }
        break;

        case MessageType::NoteOffOnly:
        {
            descriptor.signal.value   = 0;
            descriptor.signal.message = protocol::midi::MessageType::NoteOff;
        }
        break;

        case MessageType::ControlChange0Only:
        {
            descriptor.signal.value = 0;
        }
        break;

        case MessageType::ProgramChangeOffsetInc:
        {
            MidiProgram.increment_offset(descriptor.signal.value);
        }
        break;

        case MessageType::ProgramChangeOffsetDec:
        {
            MidiProgram.decrement_offset(descriptor.signal.value);
        }
        break;

        case MessageType::PresetChange:
        {
            signaling::SystemSignal event = {};
            event.system_event            = signaling::SystemEvent::PresetChangeDirectReq;
            event.value                   = descriptor.signal.index;
            signaling::publish(event);
            return;
        }
        break;

        case MessageType::BpmInc:
        {
            descriptor.signal.value = 0;

            if (!Bpm.increment(1))
            {
                send = false;
            }

            descriptor.signal.index = Bpm.value();
        }
        break;

        case MessageType::BpmDec:
        {
            descriptor.signal.value = 0;

            if (!Bpm.decrement(1))
            {
                send = false;
            }

            descriptor.signal.index = Bpm.value();
        }
        break;

        default:
        {
            send = false;
        }
        break;
        }
    }
    else
    {
        switch (descriptor.message_type)
        {
        case MessageType::Note:
        {
            descriptor.signal.value   = 0;
            descriptor.signal.message = protocol::midi::MessageType::NoteOff;
        }
        break;

        case MessageType::ControlChangeReset:
        {
            descriptor.signal.value = 0;
        }
        break;

        case MessageType::MmcRecord:
        {
            descriptor.signal.message = protocol::midi::MessageType::MmcRecordStop;
        }
        break;

        case MessageType::MmcPlayStop:
        {
            descriptor.signal.message = protocol::midi::MessageType::MmcStop;
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
        descriptor.signal.source = signaling::MidiSource::Button;
        signaling::publish(descriptor.signal);
    }
}

void Buttons::set_state(size_t index, bool state)
{
    const auto location = io::common::bit_storage_location<STATE_STORAGE_DIVISOR>(index);

    zmisc::bit_write(_button_pressed[location.array_index], location.bit_index, state);
}

bool Buttons::cached_state(size_t index)
{
    const auto location = io::common::bit_storage_location<STATE_STORAGE_DIVISOR>(index);

    return zmisc::bit_read(_button_pressed[location.array_index], location.bit_index);
}

void Buttons::set_latching_state(size_t index, bool state)
{
    const auto location = io::common::bit_storage_location<STATE_STORAGE_DIVISOR>(index);

    zmisc::bit_write(_last_latching_state[location.array_index], location.bit_index, state);
}

bool Buttons::latching_state(size_t index)
{
    const auto location = io::common::bit_storage_location<STATE_STORAGE_DIVISOR>(index);

    return zmisc::bit_read(_last_latching_state[location.array_index], location.bit_index);
}

void Buttons::reset(size_t index)
{
    set_state(index, false);
    set_latching_state(index, false);
}

void Buttons::fill_descriptor(size_t index, Descriptor& descriptor)
{
    descriptor.type                   = static_cast<Type>(_database.read(database::Config::Section::Button::Type, index));
    descriptor.message_type           = static_cast<MessageType>(_database.read(database::Config::Section::Button::MessageType, index));
    descriptor.signal.component_index = index;
    descriptor.signal.channel         = _database.read(database::Config::Section::Button::Channel, index);
    descriptor.signal.index           = _database.read(database::Config::Section::Button::MidiId, index);
    descriptor.signal.value           = _database.read(database::Config::Section::Button::Value, index);

    // overwrite type under certain conditions
    switch (descriptor.message_type)
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
    {
        descriptor.type = Type::Momentary;
    }
    break;

    case MessageType::MmcRecord:
    case MessageType::MmcPlayStop:
    {
        descriptor.type = Type::Latching;
    }
    break;

    default:
        break;
    }

    descriptor.signal.message = INTERNAL_MSG_TO_MIDI_TYPE[static_cast<uint8_t>(descriptor.message_type)];
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
