/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_LEDS

#include "leds.h"
#include "signaling/signaling.h"
#include "global/midi_program.h"
#include "util/conversion/conversion.h"
#include "util/configurable/configurable.h"

#include "zlibs/utils/misc/bit.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

using namespace opendeck;
using namespace opendeck::io::leds;
using namespace opendeck::protocol;

namespace zmidi = zlibs::utils::midi;
namespace zmisc = zlibs::utils::misc;

namespace
{
    LOG_MODULE_REGISTER(leds, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

    constexpr uint16_t STARTUP_ANIMATION_ON_DELAY_MS   = 1000;
    constexpr uint16_t STARTUP_ANIMATION_STEP_DELAY_MS = 35;
    constexpr uint8_t  MIDI_VALUE_GROUP_SIZE           = 16;
}    // namespace

Leds::Leds(Hwa&      hwa,
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

    for (size_t i = 0; i < TOTAL_BLINK_SPEEDS; i++)
    {
        _blink_state[i] = true;
    }

    for (size_t i = 0; i < Collection::size(); i++)
    {
        _brightness[i] = Brightness::Off;
    }

    signaling::subscribe<signaling::UmpSignal>(
        [this](const signaling::UmpSignal& signal)
        {
            if (is_frozen())
            {
                return;
            }

            if (signal.direction != signaling::MidiDirection::In)
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
                midi_to_state(message, signaling::MidiDirection::In);
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
                reset_blinking();
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

    signaling::subscribe<signaling::TouchscreenScreenSignal>(
        [this](const signaling::TouchscreenScreenSignal&)
        {
            if (is_frozen())
            {
                return;
            }

            const zmisc::LockGuard lock(_state_mutex);
            refresh();
            request_update(false);
        });

    signaling::subscribe<signaling::MidiSignal>(
        [this](const signaling::MidiSignal& signal)
        {
            if (is_frozen())
            {
                return;
            }

            const auto direction = (signal.source == signaling::MidiSource::Program)
                                       ? signaling::MidiDirection::In
                                       : signaling::MidiDirection::Out;

            switch (signal.source)
            {
            case signaling::MidiSource::Button:
            case signaling::MidiSource::Analog:
            case signaling::MidiSource::AnalogButton:
            case signaling::MidiSource::Encoder:
            case signaling::MidiSource::Program:
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
                direction);

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
                signaling::MidiDirection::In);

            request_update(false);
        });

    ConfigHandler.register_config(
        sys::Config::Block::Leds,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sys_config_get(static_cast<sys::Config::Section::Leds>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sys_config_set(static_cast<sys::Config::Section::Leds>(section), index, value);
        });
}

Leds::~Leds()
{
    shutdown();
}

bool Leds::init()
{
    set_all_off();

    if (_database.read(database::Config::Section::Leds::Global, Setting::UseStartupAnimation))
    {
        start_up_animation();
    }

    set_blink_type(static_cast<BlinkType>(_database.read(database::Config::Section::Leds::Global, Setting::BlinkWithMidiClock)));
    set_all_static_on();
    internal_preset_to_state(_database.current_preset());

    _thread.run();
    request_update(false);

    return true;
}

void Leds::deinit()
{
    shutdown();
}

size_t Leds::refreshable_components() const
{
    return Collection::size();
}

void Leds::force_refresh(size_t start_index, size_t count)
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
        set_state(i, _brightness[i]);
    }

    _hwa.update();
}

void Leds::shutdown()
{
    _thread.destroy();
}

void Leds::request_update(bool force_refresh)
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

bool Leds::wait_for_update()
{
    return k_sem_take(&_update_semaphore, K_FOREVER) == 0;
}

void Leds::process_update(bool force_refresh)
{
    if (is_frozen())
    {
        return;
    }

    const zmisc::LockGuard lock(_state_mutex);

    if (_blink_reset_array_ptr == nullptr)
    {
        _hwa.update();
        return;
    }

    switch (_led_blink_type)
    {
    case BlinkType::Timer:
    {
        if ((k_uptime_get_32() - _last_led_blink_update_time) < LED_BLINK_TIMER_TYPE_CHECK_TIME)
        {
            _hwa.update();
            return;
        }

        _last_led_blink_update_time = k_uptime_get_32();
    }
    break;

    case BlinkType::MidiClock:
    {
        if (!force_refresh)
        {
            _hwa.update();
            return;
        }
    }
    break;

    default:
    {
        _hwa.update();
        return;
    }
    break;
    }

    // change the blink state for specific blink rate
    for (size_t i = 0; i < TOTAL_BLINK_SPEEDS; i++)
    {
        if (++_blink_counter[i] < _blink_reset_array_ptr[i])
        {
            continue;
        }

        _blink_state[i]   = !_blink_state[i];
        _blink_counter[i] = 0;

        // assign changed state to all leds which have this speed
        for (size_t j = 0; j < Collection::size(); j++)
        {
            if (!bit(j, LedBit::BlinkOn))
            {
                continue;
            }

            if (_blink_timer[j] != i)
            {
                continue;
            }

            update_bit(j, LedBit::State, _blink_state[i]);
            set_state(j, bit(j, LedBit::State) ? _brightness[j] : Brightness::Off);
        }
    }

    _hwa.update();
}

__attribute__((weak)) void Leds::start_up_animation()
{
    // turn all leds on first
    set_all_on();

    k_msleep(STARTUP_ANIMATION_ON_DELAY_MS);

    for (size_t i = 0; i < Collection::size(GroupDigitalOutputs); i++)
    {
        set_state(i, Brightness::Off);
        k_msleep(STARTUP_ANIMATION_STEP_DELAY_MS);
    }

    for (size_t i = 0; i < Collection::size(GroupDigitalOutputs); i++)
    {
        set_state(Collection::size(GroupDigitalOutputs) - 1 - i, Brightness::Level100);
        k_msleep(STARTUP_ANIMATION_STEP_DELAY_MS);
    }

    for (size_t i = 0; i < Collection::size(GroupDigitalOutputs); i++)
    {
        set_state(i, Brightness::Off);
        k_msleep(STARTUP_ANIMATION_STEP_DELAY_MS);
    }

    // turn all off again
    set_all_off();
}

Color Leds::value_to_color(uint8_t value)
{
    // there are 7 total colors (+ off)
    return static_cast<Color>(value / MIDI_VALUE_GROUP_SIZE);
}

BlinkSpeed Leds::value_to_blink_speed(uint8_t value)
{
    if (value < MIDI_VALUE_GROUP_SIZE)
    {
        return BlinkSpeed::NoBlink;
    }

    // there are 4 total blink speeds
    return static_cast<BlinkSpeed>(value % MIDI_VALUE_GROUP_SIZE / TOTAL_BLINK_SPEEDS);
}

Brightness Leds::value_to_brightness(uint8_t value)
{
    if (value < MIDI_VALUE_GROUP_SIZE)
    {
        return Brightness::Off;
    }

    return static_cast<Brightness>((value % MIDI_VALUE_GROUP_SIZE % TOTAL_BRIGHTNESS_VALUES) + 1);
}

void Leds::midi_to_state(const protocol::midi::Message& message, signaling::MidiDirection direction)
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
        auto control_type = static_cast<ControlType>(_database.read(database::Config::Section::Leds::ControlType, i));

        // match received midi message with the assigned LED control type
        if (!is_control_type_matched(midi_message, control_type))
        {
            continue;
        }

        bool set_state     = false;
        bool set_blink     = false;
        bool check_channel = true;

        // determine whether led state or blink state should be changed
        // received MIDI message must match with defined control type
        if (direction != signaling::MidiDirection::In)
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
                    set_blink = true;
                }
            }
            break;

            case ControlType::LocalCcMultiVal:
            {
                if (midi_message == protocol::midi::MessageType::ControlChange)
                {
                    set_state = true;
                    set_blink = true;
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
                    set_blink = true;
                }
            }
            break;

            case ControlType::MidiInCcMultiVal:
            {
                if (midi_message == protocol::midi::MessageType::ControlChange)
                {
                    set_state = true;
                    set_blink = true;
                }
            }
            break;

            default:
                break;
            }
        }

        const auto db_channel = _database.read(database::Config::Section::Leds::Channel, i);
        const bool use_omni   = (use_global_channel && (global_channel == protocol::midi::OMNI_CHANNEL)) || (db_channel == protocol::midi::OMNI_CHANNEL);

        if (check_channel && !use_omni)
        {
            const auto check_channel_value = use_global_channel ? global_channel : db_channel;

            if (check_channel_value != message.channel)
            {
                continue;
            }
        }

        if (set_state || set_blink)
        {
            auto color      = Color::Off;
            auto brightness = Brightness::Off;

            // in single value modes, brightness and blink speed cannot be controlled since we're dealing
            // with one value only

            uint8_t activation_id = _database.read(database::Config::Section::Leds::ActivationId, i);

            if (midi_message == protocol::midi::MessageType::ProgramChange)
            {
                if (_database.read(database::Config::Section::Leds::Global, Setting::UseMidiProgramOffset))
                {
                    activation_id += MidiProgram.offset();
                    activation_id &= zmidi::MIDI_DATA_BYTE_MASK;
                }
            }

            if (set_state)
            {
                // match activation ID with received ID
                if (activation_id == message.data1)
                {
                    if (midi_message == protocol::midi::MessageType::ProgramChange)
                    {
                        // byte2 doesn't exist on program change message
                        color      = Color::Red;
                        brightness = Brightness::Level100;
                    }
                    else
                    {
                        // when note/cc are used to control both state and blinking ignore activation velocity
                        if (set_blink)
                        {
                            color      = value_to_color(message.data2);
                            brightness = value_to_brightness(message.data2);
                        }
                        else
                        {
                            // this has side effect that it will always set RGB LED to red color since no color information is available
                            color      = (_database.read(database::Config::Section::Leds::ActivationValue, i) == message.data2) ? Color::Red : Color::Off;
                            brightness = Brightness::Level100;
                        }
                    }

                    set_color(i, color, brightness);
                }
                else
                {
                    if (midi_message == protocol::midi::MessageType::ProgramChange)
                    {
                        set_color(i, Color::Off, Brightness::Off);
                    }
                }
            }

            if (set_blink)
            {
                // match activation ID with received ID
                if (activation_id == message.data1)
                {
                    // if both state and blink speed should be set, then don't update the state again in set_blink_speed
                    set_blink_speed(i, value_to_blink_speed(message.data2), !(set_state && set_blink));
                }
            }
        }
    }
}

void Leds::internal_preset_to_state(uint8_t preset)
{
    for (size_t i = 0; i < Collection::size(); i++)
    {
        auto control_type = static_cast<ControlType>(_database.read(database::Config::Section::Leds::ControlType, i));

        if (control_type != ControlType::Preset)
        {
            continue;
        }

        auto    color         = Color::Off;
        auto    brightness    = Brightness::Off;
        uint8_t activation_id = _database.read(database::Config::Section::Leds::ActivationId, i);

        if (_database.read(database::Config::Section::Leds::Global, Setting::UseMidiProgramOffset))
        {
            activation_id += MidiProgram.offset();
            activation_id &= zmidi::MIDI_DATA_BYTE_MASK;
        }

        if (activation_id == preset)
        {
            color      = Color::Red;
            brightness = Brightness::Level100;
        }

        set_color(i, color, brightness);
    }
}

void Leds::set_blink_speed(uint8_t index, BlinkSpeed state, bool update_state)
{
    uint8_t led_array[3]    = {};
    uint8_t leds            = 0;
    uint8_t rgb_from_output = _hwa.rgb_from_output(index);

    if (_database.read(database::Config::Section::Leds::RgbEnable, rgb_from_output))
    {
        led_array[0] = _hwa.rgb_component_from_rgb(rgb_from_output, RgbComponent::R);
        led_array[1] = _hwa.rgb_component_from_rgb(rgb_from_output, RgbComponent::G);
        led_array[2] = _hwa.rgb_component_from_rgb(rgb_from_output, RgbComponent::B);

        leds = 3;
    }
    else
    {
        led_array[0] = index;

        leds = 1;
    }

    for (int i = 0; i < leds; i++)
    {
        if (state != BlinkSpeed::NoBlink)
        {
            update_bit(led_array[i], LedBit::BlinkOn, true);
            update_bit(led_array[i], LedBit::State, true);
        }
        else
        {
            update_bit(led_array[i], LedBit::BlinkOn, false);
            update_bit(led_array[i], LedBit::State, bit(led_array[i], LedBit::Active));
        }

        _blink_timer[index] = static_cast<uint8_t>(state);

        if (update_state)
        {
            set_state(led_array[i], _brightness[led_array[i]]);
        }
    }
}

void Leds::set_all_on()
{
    // turn on all Leds
    for (size_t i = 0; i < Collection::size(); i++)
    {
        set_color(i, Color::Red, Brightness::Level100);
    }
}

void Leds::set_all_static_on()
{
    // turn on all static Leds
    for (size_t i = 0; i < Collection::size(); i++)
    {
        if (_database.read(database::Config::Section::Leds::ControlType, i) == static_cast<uint8_t>(ControlType::Static))
        {
            set_color(i, Color::Red, Brightness::Level100);
        }
    }
}

void Leds::set_all_off()
{
    // turn off all Leds
    for (size_t i = 0; i < Collection::size(); i++)
    {
        reset_state(i);
    }
}

void Leds::refresh()
{
    for (size_t i = 0; i < Collection::size(); i++)
    {
        set_state(i, _brightness[i]);
    }
}

void Leds::set_color(uint8_t index, Color color, Brightness brightness)
{
    uint8_t rgb_from_output = _hwa.rgb_from_output(index);

    auto handle_led = [&](uint8_t led_index, RgbComponent rgb_component, bool state, bool is_rgb)
    {
        if (state)
        {
            update_bit(led_index, LedBit::Active, true);
            update_bit(led_index, LedBit::State, true);

            if (is_rgb)
            {
                update_bit(led_index, LedBit::Rgb, true);

                switch (rgb_component)
                {
                case RgbComponent::R:
                {
                    update_bit(led_index, LedBit::RgbR, state);
                }
                break;

                case RgbComponent::G:
                {
                    update_bit(led_index, LedBit::RgbG, state);
                }
                break;

                case RgbComponent::B:
                {
                    update_bit(led_index, LedBit::RgbB, state);
                }
                break;
                }
            }
            else
            {
                update_bit(led_index, LedBit::Rgb, false);
            }

            _brightness[led_index] = brightness;
            set_state(led_index, brightness);
        }
        else
        {
            // turn off the led
            reset_state(led_index);
        }
    };

    if (_database.read(database::Config::Section::Leds::RgbEnable, rgb_from_output))
    {
        // rgb led is composed of three standard Leds
        // get indexes of individual Leds first
        uint8_t red_led   = _hwa.rgb_component_from_rgb(rgb_from_output, RgbComponent::R);
        uint8_t green_led = _hwa.rgb_component_from_rgb(rgb_from_output, RgbComponent::G);
        uint8_t blue_led  = _hwa.rgb_component_from_rgb(rgb_from_output, RgbComponent::B);

        handle_led(red_led, RgbComponent::R, zmisc::bit_read(static_cast<uint8_t>(color), static_cast<size_t>(RgbComponent::R)), true);
        handle_led(green_led, RgbComponent::G, zmisc::bit_read(static_cast<uint8_t>(color), static_cast<size_t>(RgbComponent::G)), true);
        handle_led(blue_led, RgbComponent::B, zmisc::bit_read(static_cast<uint8_t>(color), static_cast<size_t>(RgbComponent::B)), true);
    }
    else
    {
        handle_led(index, RgbComponent::R, static_cast<bool>(color), false);
    }
}

BlinkSpeed Leds::blink_speed(uint8_t index)
{
    if (!bit(index, LedBit::BlinkOn))
    {
        return BlinkSpeed::NoBlink;
    }

    return static_cast<BlinkSpeed>(_blink_timer[index]);
}

void Leds::set_blink_type(BlinkType blink_type)
{
    switch (blink_type)
    {
    case BlinkType::Timer:
    {
        _blink_reset_array_ptr = BLINK_RESET_TIMER;
    }
    break;

    case BlinkType::MidiClock:
    {
        _blink_reset_array_ptr = BLINK_RESET_MIDI_CLOCK;
    }
    break;

    default:
        return;
    }

    _led_blink_type = blink_type;
}

void Leds::reset_blinking()
{
    // reset all counters in this case
    // also make sure all leds are in sync again
    for (size_t i = 0; i < TOTAL_BLINK_SPEEDS; i++)
    {
        _blink_counter[i] = 0;
        _blink_state[i]   = true;
    }
}

void Leds::update_bit(uint8_t index, LedBit bit, bool state)
{
    zmisc::bit_write(_led_state[index], static_cast<uint8_t>(bit), state);
}

bool Leds::bit(uint8_t index, LedBit bit)
{
    return zmisc::bit_read(_led_state[index], static_cast<size_t>(bit));
}

Color Leds::color(uint8_t index)
{
    if (!bit(index, LedBit::Active))
    {
        return Color::Off;
    }

    if (!bit(index, LedBit::Rgb))
    {
        // single color led
        return Color::Red;
    }

    // rgb led
    uint8_t rgb_from_output = _hwa.rgb_from_output(index);

    uint8_t color = 0;
    color |= bit(_hwa.rgb_component_from_rgb(rgb_from_output, RgbComponent::B), LedBit::RgbB);
    color <<= 1;
    color |= bit(_hwa.rgb_component_from_rgb(rgb_from_output, RgbComponent::G), LedBit::RgbG);
    color <<= 1;
    color |= bit(_hwa.rgb_component_from_rgb(rgb_from_output, RgbComponent::R), LedBit::RgbR);

    return static_cast<Color>(color);
}

void Leds::reset_state(uint8_t index)
{
    _led_state[index]  = 0;
    _brightness[index] = Brightness::Off;
    set_state(index, Brightness::Off);
}

void Leds::set_state(size_t index, Brightness brightness)
{
    if (index >= Collection::size(GroupDigitalOutputs))
    {
        // specified hwa interface only writes to physical leds
        // for touchscreen and other destinations, notify via dispatcher

        signaling::TouchscreenLedSignal signal = {};
        signal.component_index                 = index - Collection::start_index(GroupTouchscreenComponents);
        signal.value                           = static_cast<uint16_t>(brightness);

        signaling::publish(signal);
    }
    else
    {
        _hwa.set_state(index, brightness);
    }
}

std::optional<uint8_t> Leds::sys_config_get(sys::Config::Section::Leds section, size_t index, uint16_t& value)
{
    uint32_t read_value = 0;
    auto     result     = sys::Config::Status::Ack;

    switch (section)
    {
    case sys::Config::Section::Leds::TestColor:
    {
        read_value = static_cast<uint32_t>(color(index));
    }
    break;

    case sys::Config::Section::Leds::Channel:
    {
        result = _database.read(util::Conversion::sys_2_db_section(section),
                                index,
                                read_value)
                     ? sys::Config::Status::Ack
                     : sys::Config::Status::ErrorRead;
    }
    break;

    case sys::Config::Section::Leds::RgbEnable:
    {
        result = _database.read(util::Conversion::sys_2_db_section(section),
                                _hwa.rgb_from_output(index),
                                read_value)
                     ? sys::Config::Status::Ack
                     : sys::Config::Status::ErrorRead;
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

std::optional<uint8_t> Leds::sys_config_set(sys::Config::Section::Leds section, size_t index, uint16_t value)
{
    uint8_t result = sys::Config::Status::ErrorWrite;

    switch (section)
    {
    case sys::Config::Section::Leds::TestColor:
    {
        // no writing to database
        set_color(index, static_cast<Color>(value), Brightness::Level100);
        result = sys::Config::Status::Ack;
    }
    break;

    case sys::Config::Section::Leds::Global:
    {
        auto led_setting = static_cast<Setting>(index);

        switch (led_setting)
        {
        case Setting::BlinkWithMidiClock:
        {
            if ((value <= 1) && (value >= 0))
            {
                result = sys::Config::Status::Ack;
                set_blink_type(static_cast<BlinkType>(value));
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

    case sys::Config::Section::Leds::RgbEnable:
    {
        // make sure to turn all three leds off before setting new state
        set_color(_hwa.rgb_component_from_rgb(_hwa.rgb_from_output(index), RgbComponent::R), Color::Off, Brightness::Off);
        set_color(_hwa.rgb_component_from_rgb(_hwa.rgb_from_output(index), RgbComponent::G), Color::Off, Brightness::Off);
        set_color(_hwa.rgb_component_from_rgb(_hwa.rgb_from_output(index), RgbComponent::B), Color::Off, Brightness::Off);

        // write rgb enabled bit to led
        result = _database.update(util::Conversion::sys_2_db_section(section),
                                  _hwa.rgb_from_output(index),
                                  value)
                     ? sys::Config::Status::Ack
                     : sys::Config::Status::ErrorWrite;

        if (value && (result == sys::Config::Status::Ack))
        {
            // copy over note activation local control and midi channel settings to all three leds from the current led index

            for (int i = 0; i < 3; i++)
            {
                result = _database.update(util::Conversion::sys_2_db_section(sys::Config::Section::Leds::ActivationId),
                                          _hwa.rgb_component_from_rgb(_hwa.rgb_from_output(index), static_cast<RgbComponent>(i)),
                                          _database.read(util::Conversion::sys_2_db_section(sys::Config::Section::Leds::ActivationId), index))
                             ? sys::Config::Status::Ack
                             : sys::Config::Status::ErrorWrite;

                if (result != sys::Config::Status::Ack)
                {
                    break;
                }

                result = _database.update(util::Conversion::sys_2_db_section(sys::Config::Section::Leds::ControlType),
                                          _hwa.rgb_component_from_rgb(_hwa.rgb_from_output(index), static_cast<RgbComponent>(i)),
                                          _database.read(util::Conversion::sys_2_db_section(sys::Config::Section::Leds::ControlType), index))
                             ? sys::Config::Status::Ack
                             : sys::Config::Status::ErrorWrite;

                if (result != sys::Config::Status::Ack)
                {
                    break;
                }

                result = _database.update(util::Conversion::sys_2_db_section(sys::Config::Section::Leds::Channel),
                                          _hwa.rgb_component_from_rgb(_hwa.rgb_from_output(index), static_cast<RgbComponent>(i)),
                                          _database.read(util::Conversion::sys_2_db_section(sys::Config::Section::Leds::Channel), index))
                             ? sys::Config::Status::Ack
                             : sys::Config::Status::ErrorWrite;

                if (result != sys::Config::Status::Ack)
                {
                    break;
                }
            }
        }
    }
    break;

    case sys::Config::Section::Leds::ActivationId:
    case sys::Config::Section::Leds::ControlType:
    case sys::Config::Section::Leds::Channel:
    {
        // first, turn the led off if control type is being changed
        if (section == sys::Config::Section::Leds::ControlType)
        {
            set_color(index,
                      value == static_cast<uint8_t>(ControlType::Static) ? Color::Red : Color::Off,
                      value == static_cast<uint8_t>(ControlType::Static) ? Brightness::Level100 : Brightness::Off);
        }

        // find out if RGB led is enabled for this led index
        if (_database.read(util::Conversion::sys_2_db_section(sys::Config::Section::Leds::RgbEnable), _hwa.rgb_from_output(index)))
        {
            // rgb led enabled - copy these settings to all three leds
            for (int i = 0; i < 3; i++)
            {
                result = _database.update(util::Conversion::sys_2_db_section(section),
                                          _hwa.rgb_component_from_rgb(_hwa.rgb_from_output(index), static_cast<RgbComponent>(i)),
                                          value)
                             ? sys::Config::Status::Ack
                             : sys::Config::Status::ErrorWrite;

                if (result != sys::Config::Status::Ack)
                {
                    break;
                }
            }
        }
        else
        {
            // apply to single led only
            result = _database.update(util::Conversion::sys_2_db_section(section), index, value)
                         ? sys::Config::Status::Ack
                         : sys::Config::Status::ErrorWrite;
        }
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
