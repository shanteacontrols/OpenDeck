/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_OUTPUTS

#include "firmware/src/io/outputs/instance/impl/outputs.h"
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/global/midi_program.h"
#include "firmware/src/util/conversion/conversion.h"
#include "firmware/src/util/configurable/configurable.h"

#include "zlibs/utils/misc/bit.h"
#include "zlibs/utils/misc/numeric.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

using namespace opendeck;
using namespace opendeck::io::outputs;
using namespace opendeck::protocol;

namespace zmidi = zlibs::utils::midi;
namespace zmisc = zlibs::utils::misc;

namespace
{
    LOG_MODULE_REGISTER(outputs, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

    constexpr uint16_t STARTUP_ANIMATION_ON_DELAY_MS   = 1000;
    constexpr uint16_t STARTUP_ANIMATION_STEP_DELAY_MS = 35;
    constexpr uint8_t  MIDI_VALUE_GROUP_SIZE           = 16;
}    // namespace

Outputs::Outputs(Hwa&      hwa,
                 Database& database)
    : _hwa(hwa)
    , _database(database)
    , _thread([&]()
              {
                  while (1)
                  {
                      wait_until_running();

                      if (!wait_for_update())
                      {
                          continue;
                      }

                      process_update(_force_refresh_pending.exchange(false, std::memory_order_acq_rel));
                  }
              })
{
    k_sem_init(&_update_semaphore, 0, K_SEM_MAX_LIMIT);

    for (size_t i = 0; i < TOTAL_PULSE_SPEEDS; i++)
    {
        _pulse_state[i] = true;
    }

    for (size_t i = 0; i < Collection::size(); i++)
    {
        _level[i] = OUTPUT_LEVEL_MIN;
    }

    signaling::subscribe<signaling::UmpSignal>(
        [this](const signaling::UmpSignal& signal)
        {
            if (is_frozen())
            {
                return;
            }

            if (signal.direction != signaling::SignalDirection::In)
            {
                return;
            }

            const auto message = protocol::midi::decode_message(signal.packet);

            switch (message.type)
            {
            case protocol::midi::MessageType::NoteOn:
            case protocol::midi::MessageType::NoteOff:
            case protocol::midi::MessageType::ControlChange:
            case protocol::midi::MessageType::ProgramChange:
            {
                const zmisc::LockGuard lock(_state_mutex);
                midi_to_state(message, signaling::SignalDirection::In);
                request_update(false);
            }
            break;

            case protocol::midi::MessageType::SysRealTimeClock:
            {
                request_update(true);
            }
            break;

            case protocol::midi::MessageType::SysRealTimeStart:
            {
                const zmisc::LockGuard lock(_state_mutex);
                reset_pulsing();
                request_update(true);
            }
            break;

            default:
                break;
            }
        });

    signaling::subscribe<signaling::SystemSignal>(
        [this](const signaling::SystemSignal& signal)
        {
            if (is_frozen())
            {
                return;
            }

            switch (signal.system_event)
            {
            case signaling::SystemEvent::PresetChanged:
            {
                const zmisc::LockGuard lock(_state_mutex);
                set_all_off();
                internal_preset_to_state(signal.value);
                request_update(false);
            }
            break;

            default:
                break;
            }
        });

    signaling::subscribe<signaling::TouchscreenScreenChangedSignal>(
        [this](const signaling::TouchscreenScreenChangedSignal&)
        {
            if (is_frozen())
            {
                return;
            }

            const zmisc::LockGuard lock(_state_mutex);

            constexpr size_t START_INDEX = Collection::start_index(GroupTouchscreenComponents);
            constexpr size_t COUNT       = Collection::size(GroupTouchscreenComponents);

            for (size_t i = START_INDEX; i < START_INDEX + COUNT; i++)
            {
                write_level(i, _level[i]);
            }

            request_update(false);
        });

    signaling::subscribe<signaling::OscIoSignal>(
        [this](const signaling::OscIoSignal& signal)
        {
            if (is_frozen())
            {
                return;
            }

            if (signal.direction != signaling::SignalDirection::In)
            {
                return;
            }

            if (signal.source != signaling::IoEventSource::Output)
            {
                return;
            }

            if (signal.component_index >= Collection::size())
            {
                return;
            }

            const zmisc::LockGuard lock(_state_mutex);
            const auto             value = signal.int32_value.value_or(0);

            set_level(static_cast<uint8_t>(signal.component_index),
                      value != 0 ? OUTPUT_LEVEL_MAX : OUTPUT_LEVEL_MIN);
            request_update(false);
        });

    signaling::subscribe<signaling::MidiIoSignal>(
        [this](const signaling::MidiIoSignal& signal)
        {
            if (is_frozen())
            {
                return;
            }

            switch (signal.source)
            {
            case signaling::IoEventSource::Switch:
            case signaling::IoEventSource::Analog:
            case signaling::IoEventSource::AnalogSwitch:
            case signaling::IoEventSource::Encoder:
                break;

            default:
                return;
            }

            const zmisc::LockGuard lock(_state_mutex);

            midi_to_state(
                {
                    .type    = signal.message,
                    .channel = signal.channel,
                    .data1   = signal.index,
                    .data2   = signal.value,
                },
                signaling::SignalDirection::Out);

            request_update(false);
        });

    signaling::subscribe<signaling::InternalProgram>(
        [this](const signaling::InternalProgram& signal)
        {
            if (is_frozen())
            {
                return;
            }

            const zmisc::LockGuard lock(_state_mutex);

            midi_to_state(
                {
                    .type    = protocol::midi::MessageType::ProgramChange,
                    .channel = signal.channel,
                    .data1   = signal.index,
                    .data2   = signal.value,
                },
                signaling::SignalDirection::In);

            request_update(false);
        });

    ConfigHandler.register_config(
        sys::Config::Block::Outputs,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sys_config_get(static_cast<sys::Config::Section::Outputs>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sys_config_set(static_cast<sys::Config::Section::Outputs>(section), index, value);
        });
}

Outputs::~Outputs()
{
    shutdown();
}

bool Outputs::init()
{
    set_all_off();

    if (_database.read(database::Config::Section::Outputs::Global, Setting::UseStartupAnimation))
    {
        start_up_animation();
    }

    set_pulse_mode(static_cast<PulseMode>(_database.read(database::Config::Section::Outputs::Global, Setting::PulseWithMidiClock)));
    set_all_static_on();
    internal_preset_to_state(_database.current_preset());

    _thread.run();
    request_update(false);

    return true;
}

void Outputs::deinit()
{
    shutdown();
}

size_t Outputs::refreshable_components() const
{
    return Collection::size();
}

void Outputs::force_refresh(size_t start_index, size_t count)
{
    const auto total = Collection::size();

    if (start_index >= total)
    {
        return;
    }

    const auto end = std::min(start_index + count, total);

    const zmisc::LockGuard lock(_state_mutex);

    for (size_t i = start_index; i < end; i++)
    {
        write_level(i, _level[i]);
    }
}

void Outputs::shutdown()
{
    _thread.destroy();
}

void Outputs::request_update(bool force_refresh)
{
    if (is_frozen())
    {
        return;
    }

    if (force_refresh)
    {
        _force_refresh_pending.store(true, std::memory_order_release);
    }

    k_sem_give(&_update_semaphore);
}

bool Outputs::wait_for_update()
{
    return k_sem_take(&_update_semaphore, K_FOREVER) == 0;
}

void Outputs::process_update(bool force_refresh)
{
    if (is_frozen())
    {
        return;
    }

    const zmisc::LockGuard lock(_state_mutex);

    if (_pulse_reset_array_ptr == nullptr)
    {
        return;
    }

    switch (_output_pulse_mode)
    {
    case PulseMode::Timer:
    {
        if ((k_uptime_get_32() - _last_output_pulse_update_time) < OUTPUT_PULSE_TIMER_MODE_CHECK_TIME)
        {
            return;
        }

        _last_output_pulse_update_time = k_uptime_get_32();
    }
    break;

    case PulseMode::MidiClock:
    {
        if (!force_refresh)
        {
            return;
        }
    }
    break;

    default:
    {
        return;
    }
    break;
    }

    // change the pulse state for specific pulse rate
    for (size_t i = 0; i < TOTAL_PULSE_SPEEDS; i++)
    {
        if (++_pulse_counter[i] < _pulse_reset_array_ptr[i])
        {
            continue;
        }

        _pulse_state[i]   = !_pulse_state[i];
        _pulse_counter[i] = 0;

        // assign changed state to all outputs which have this speed
        for (size_t j = 0; j < Collection::size(); j++)
        {
            if (!bit(j, OutputBit::PulseOn))
            {
                continue;
            }

            if (_pulse_timer[j] != i)
            {
                continue;
            }

            update_bit(j, OutputBit::State, _pulse_state[i]);
            write_level(j, bit(j, OutputBit::State) ? _level[j] : OUTPUT_LEVEL_MIN);
        }
    }
}

void Outputs::start_up_animation()
{
    // turn all outputs on first
    set_all_on();

    k_msleep(STARTUP_ANIMATION_ON_DELAY_MS);

    for (size_t i = 0; i < Collection::size(GroupDigitalOutputs); i++)
    {
        set_level(i, OUTPUT_LEVEL_MIN);
        k_msleep(STARTUP_ANIMATION_STEP_DELAY_MS);
    }

    for (size_t i = 0; i < Collection::size(GroupDigitalOutputs); i++)
    {
        set_level(Collection::size(GroupDigitalOutputs) - 1 - i, OUTPUT_LEVEL_MAX);
        k_msleep(STARTUP_ANIMATION_STEP_DELAY_MS);
    }

    for (size_t i = 0; i < Collection::size(GroupDigitalOutputs); i++)
    {
        set_level(i, OUTPUT_LEVEL_MIN);
        k_msleep(STARTUP_ANIMATION_STEP_DELAY_MS);
    }

    // turn all off again
    set_all_off();
}

PulseSpeed Outputs::value_to_pulse_speed(uint8_t value)
{
    if (value < MIDI_VALUE_GROUP_SIZE)
    {
        return PulseSpeed::NoPulse;
    }

    // there are 4 total pulse speeds
    return static_cast<PulseSpeed>(value % MIDI_VALUE_GROUP_SIZE / TOTAL_PULSE_SPEEDS);
}

uint8_t Outputs::value_to_level(uint8_t value)
{
    return static_cast<uint8_t>(zmisc::map_range(static_cast<uint32_t>(value),
                                                 0U,
                                                 static_cast<uint32_t>(protocol::midi::MAX_VALUE_7BIT),
                                                 static_cast<uint32_t>(OUTPUT_LEVEL_MIN),
                                                 static_cast<uint32_t>(OUTPUT_LEVEL_MAX)));
}

void Outputs::midi_to_state(const protocol::midi::Message& message, signaling::SignalDirection direction)
{
    const uint8_t global_channel     = _database.read(database::Config::Section::Global::MidiSettings, protocol::midi::Setting::GlobalChannel);
    const uint8_t use_global_channel = _database.read(database::Config::Section::Global::MidiSettings,
                                                      protocol::midi::Setting::UseGlobalChannel);

    auto is_control_type_matched = [](protocol::midi::MessageType midi_message, ControlType control_type)
    {
        return CONTROL_TYPE_TO_MIDI_MESSAGE[static_cast<uint8_t>(control_type)] == midi_message;
    };

    auto midi_message = message.type;

    // ignore the distinction between note on and off
    if (midi_message == protocol::midi::MessageType::NoteOff)
    {
        midi_message = protocol::midi::MessageType::NoteOn;
    }

    for (size_t i = 0; i < Collection::size(); i++)
    {
        auto control_type = static_cast<ControlType>(_database.read(database::Config::Section::Outputs::ControlType, i));

        // match received midi message with the assigned output control type
        if (!is_control_type_matched(midi_message, control_type))
        {
            continue;
        }

        bool set_state     = false;
        bool set_pulse     = false;
        bool check_channel = true;

        // determine whether output state or pulse state should be changed
        // received MIDI message must match with defined control type
        if (direction != signaling::SignalDirection::In)
        {
            switch (control_type)
            {
            case ControlType::LocalNoteSingleVal:
            {
                if (midi_message == protocol::midi::MessageType::NoteOn)
                {
                    set_state = true;
                }
            }
            break;

            case ControlType::LocalCcSingleVal:
            {
                if (midi_message == protocol::midi::MessageType::ControlChange)
                {
                    set_state = true;
                }
            }
            break;

            case ControlType::PcSingleVal:
            {
                if (midi_message == protocol::midi::MessageType::ProgramChange)
                {
                    set_state = true;
                }
            }
            break;

            case ControlType::Preset:
            {
                check_channel = false;
            }
            break;

            case ControlType::LocalNoteMultiVal:
            {
                if (midi_message == protocol::midi::MessageType::NoteOn)
                {
                    set_state = true;
                    set_pulse = true;
                }
            }
            break;

            case ControlType::LocalCcMultiVal:
            {
                if (midi_message == protocol::midi::MessageType::ControlChange)
                {
                    set_state = true;
                    set_pulse = true;
                }
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
                if (midi_message == protocol::midi::MessageType::NoteOn)
                {
                    set_state = true;
                }
            }
            break;

            case ControlType::MidiInCcSingleVal:
            {
                if (midi_message == protocol::midi::MessageType::ControlChange)
                {
                    set_state = true;
                }
            }
            break;

            case ControlType::PcSingleVal:
            {
                if (midi_message == protocol::midi::MessageType::ProgramChange)
                {
                    set_state = true;
                }
            }
            break;

            case ControlType::MidiInNoteMultiVal:
            {
                if (midi_message == protocol::midi::MessageType::NoteOn)
                {
                    set_state = true;
                    set_pulse = true;
                }
            }
            break;

            case ControlType::MidiInCcMultiVal:
            {
                if (midi_message == protocol::midi::MessageType::ControlChange)
                {
                    set_state = true;
                    set_pulse = true;
                }
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

        if (set_state || set_pulse)
        {
            auto level = OUTPUT_LEVEL_MIN;

            // in single value modes, level and pulse speed cannot be controlled since we're dealing
            // with one value only

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
                set_pulse_speed(i, value_to_pulse_speed(message.data2));
            }

            if (set_state)
            {
                // match activation ID with received ID
                if (activation_id_matches)
                {
                    if (midi_message == protocol::midi::MessageType::ProgramChange)
                    {
                        // byte2 doesn't exist on program change message
                        level = OUTPUT_LEVEL_MAX;
                    }
                    else
                    {
                        // when note/cc are used to control both state and pulsing ignore activation velocity
                        if (set_pulse)
                        {
                            level = value_to_level(message.data2);
                        }
                        else
                        {
                            level = (_database.read(database::Config::Section::Outputs::ActivationValue, i) == message.data2) ? OUTPUT_LEVEL_MAX : OUTPUT_LEVEL_MIN;
                        }
                    }

                    set_level(i, level);
                }
                else
                {
                    if (midi_message == protocol::midi::MessageType::ProgramChange)
                    {
                        set_level(i, OUTPUT_LEVEL_MIN);
                    }
                }
            }
            else if (set_pulse && activation_id_matches)
            {
                // Pulse-only controls change timing, then write the current level.
                write_level(i, _level[i]);
            }
        }
    }
}

void Outputs::internal_preset_to_state(uint8_t preset)
{
    for (size_t i = 0; i < Collection::size(); i++)
    {
        auto control_type = static_cast<ControlType>(_database.read(database::Config::Section::Outputs::ControlType, i));

        if (control_type != ControlType::Preset)
        {
            continue;
        }

        auto    level         = OUTPUT_LEVEL_MIN;
        uint8_t activation_id = _database.read(database::Config::Section::Outputs::ActivationId, i);

        if (_database.read(database::Config::Section::Outputs::Global, Setting::UseMidiProgramOffset))
        {
            activation_id += MidiProgram.offset();
            activation_id &= zmidi::MIDI_DATA_BYTE_MASK;
        }

        if (activation_id == preset)
        {
            level = OUTPUT_LEVEL_MAX;
        }

        set_level(i, level);
    }
}

void Outputs::set_pulse_speed(size_t index, PulseSpeed state)
{
    if (state != PulseSpeed::NoPulse)
    {
        update_bit(index, OutputBit::PulseOn, true);
        update_bit(index, OutputBit::State, true);
    }
    else
    {
        update_bit(index, OutputBit::PulseOn, false);
        update_bit(index, OutputBit::State, bit(index, OutputBit::Active));
    }

    _pulse_timer[index] = static_cast<uint8_t>(state);
}

void Outputs::set_all_on()
{
    // turn on all outputs
    for (size_t i = 0; i < Collection::size(); i++)
    {
        set_level(i, OUTPUT_LEVEL_MAX);
    }
}

void Outputs::set_all_static_on()
{
    // turn on all static outputs
    for (size_t i = 0; i < Collection::size(); i++)
    {
        if (_database.read(database::Config::Section::Outputs::ControlType, i) == static_cast<uint8_t>(ControlType::Static))
        {
            set_level(i, OUTPUT_LEVEL_MAX);
        }
    }
}

void Outputs::set_all_off()
{
    // turn off all outputs
    for (size_t i = 0; i < Collection::size(); i++)
    {
        reset_state(i);
    }
}

PulseSpeed Outputs::pulse_speed(size_t index)
{
    if (!bit(index, OutputBit::PulseOn))
    {
        return PulseSpeed::NoPulse;
    }

    return static_cast<PulseSpeed>(_pulse_timer[index]);
}

void Outputs::set_pulse_mode(PulseMode pulse_mode)
{
    switch (pulse_mode)
    {
    case PulseMode::Timer:
    {
        _pulse_reset_array_ptr = PULSE_RESET_TIMER;
    }
    break;

    case PulseMode::MidiClock:
    {
        _pulse_reset_array_ptr = PULSE_RESET_MIDI_CLOCK;
    }
    break;

    default:
        return;
    }

    _output_pulse_mode = pulse_mode;
}

void Outputs::reset_pulsing()
{
    // reset all counters in this case
    // also make sure all outputs are in sync again
    for (size_t i = 0; i < TOTAL_PULSE_SPEEDS; i++)
    {
        _pulse_counter[i] = 0;
        _pulse_state[i]   = true;
    }
}

void Outputs::update_bit(size_t index, OutputBit bit, bool state)
{
    zmisc::bit_write(_output_state[index], static_cast<uint8_t>(bit), state);
}

bool Outputs::bit(size_t index, OutputBit bit)
{
    return zmisc::bit_read(_output_state[index], static_cast<size_t>(bit));
}

void Outputs::reset_state(size_t index)
{
    _output_state[index] = 0;
    _level[index]        = OUTPUT_LEVEL_MIN;
    write_level(index, OUTPUT_LEVEL_MIN);
}

void Outputs::set_level(size_t index, uint8_t level)
{
    if (level > OUTPUT_LEVEL_MIN)
    {
        update_bit(index, OutputBit::Active, true);
        update_bit(index, OutputBit::State, true);

        _level[index] = level;
    }
    else
    {
        reset_state(index);
        return;
    }

    write_level(index, level);
}

void Outputs::write_level(size_t index, uint8_t level)
{
    signaling::publish(signaling::OscIoSignal{
        .source          = signaling::IoEventSource::Output,
        .component_index = index,
        .int32_value     = static_cast<int32_t>(level),
        .direction       = signaling::SignalDirection::Out,
    });

    if (index < Collection::size(GroupDigitalOutputs))
    {
        _hwa.set_level(index, level);
    }
}

std::optional<uint8_t> Outputs::sys_config_get(sys::Config::Section::Outputs section, size_t index, uint16_t& value)
{
    uint32_t read_value = 0;
    auto     result     = sys::Config::Status::Ack;

    switch (section)
    {
    case sys::Config::Section::Outputs::State:
    {
        read_value = bit(index, OutputBit::Active) ? 1 : 0;
        result     = sys::Config::Status::Ack;
    }
    break;

    default:
    {
        result = _database.read(util::Conversion::sys_2_db_section(section),
                                index,
                                read_value)
                     ? sys::Config::Status::Ack
                     : sys::Config::Status::ErrorRead;
    }
    break;
    }

    value = read_value;

    return result;
}

std::optional<uint8_t> Outputs::sys_config_set(sys::Config::Section::Outputs section, size_t index, uint16_t value)
{
    uint8_t result = sys::Config::Status::ErrorWrite;

    switch (section)
    {
    case sys::Config::Section::Outputs::State:
    {
        set_level(index, value ? OUTPUT_LEVEL_MAX : OUTPUT_LEVEL_MIN);
        result = sys::Config::Status::Ack;
    }
    break;

    case sys::Config::Section::Outputs::Global:
    {
        auto output_setting = static_cast<Setting>(index);

        switch (output_setting)
        {
        case Setting::PulseWithMidiClock:
        {
            if ((value <= 1) && (value >= 0))
            {
                result = sys::Config::Status::Ack;
                set_pulse_mode(static_cast<PulseMode>(value));
            }
        }
        break;

        case Setting::UseStartupAnimation:
        {
            if ((value <= 1) && (value >= 0))
            {
                result = sys::Config::Status::Ack;
            }
        }
        break;

        case Setting::UseMidiProgramOffset:
        case Setting::Unused:
        {
            result = sys::Config::Status::Ack;
        }
        break;

        default:
            break;
        }

        // write to db if success is true and writing should take place
        if (result == sys::Config::Status::Ack)
        {
            result = _database.update(util::Conversion::sys_2_db_section(section), index, value)
                         ? sys::Config::Status::Ack
                         : sys::Config::Status::ErrorWrite;
        }
    }
    break;

    case sys::Config::Section::Outputs::ActivationId:
    case sys::Config::Section::Outputs::ControlType:
    case sys::Config::Section::Outputs::Channel:
    {
        // first, turn the output off if control type is being changed
        if (section == sys::Config::Section::Outputs::ControlType)
        {
            set_level(index,
                      value == static_cast<uint8_t>(ControlType::Static) ? OUTPUT_LEVEL_MAX : OUTPUT_LEVEL_MIN);
        }

        result = _database.update(util::Conversion::sys_2_db_section(section), index, value)
                     ? sys::Config::Status::Ack
                     : sys::Config::Status::ErrorWrite;
    }
    break;

    default:
    {
        result = _database.update(util::Conversion::sys_2_db_section(section), index, value)
                     ? sys::Config::Status::Ack
                     : sys::Config::Status::ErrorWrite;
    }
    break;
    }

    return result;
}

#endif
