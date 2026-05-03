/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ADC

#include "analog.h"
#include "remap.h"
#include "system/config.h"
#include "util/conversion/conversion.h"
#include "util/configurable/configurable.h"
#include "util/thread_sleep.h"

#include "zlibs/utils/misc/bit.h"
#include "zlibs/utils/misc/numeric.h"
#include "zlibs/utils/misc/mutex.h"

#include <zephyr/logging/log.h>

using namespace opendeck;
using namespace opendeck::io::analog;
using namespace opendeck::protocol;

namespace
{
    LOG_MODULE_REGISTER(analog, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

    constexpr uint16_t RESET_ANALOG_VALUE = 0xFFFF;
}    // namespace

Analog::Analog(Hwa&      hwa,
               Filter&   filter,
               Database& database)
    : _hwa(hwa)
    , _filter(filter)
    , _database(database)
    , _thread([&]()
              {
                  while (1)
                  {
                      wait_until_running();

                      if (is_frozen())
                      {
                          util::thread_sleep(CONFIG_ANALOG_THREAD_SLEEP_MS);
                          continue;
                      }

                      auto frame = _hwa.read();

                      if (frame.has_value())
                      {
                          const zlibs::utils::misc::LockGuard lock(_state_mutex);
                          process_frame(frame.value());
                      }

                      util::thread_sleep(CONFIG_ANALOG_THREAD_SLEEP_MS);
                  }
              })
{
    ConfigHandler.register_config(
        sys::Config::Block::Analog,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sys_config_get(static_cast<sys::Config::Section::Analog>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sys_config_set(static_cast<sys::Config::Section::Analog>(section), index, value);
        });
}

Analog::~Analog()
{
    _thread.destroy();
    _hwa.deinit();
}

bool Analog::init()
{
    if (!_hwa.init())
    {
        return false;
    }

    for (size_t i = 0; i < Collection::size(); i++)
    {
        reset(i);
    }

    _thread.run();

    return true;
}

void Analog::deinit()
{
    _thread.destroy();
    _hwa.deinit();
}

size_t Analog::refreshable_components() const
{
    return Collection::size(GroupAnalogInputs);
}

void Analog::force_refresh(size_t start_index, size_t count)
{
    const auto total = Collection::size(GroupAnalogInputs);

    if (start_index >= total)
    {
        return;
    }

    const auto end = std::min(start_index + count, total);

    const zlibs::utils::misc::LockGuard lock(_state_mutex);

    for (size_t i = start_index; i < end; i++)
    {
        if (_database.read(database::Config::Section::Analog::Enable, i))
        {
            Descriptor descriptor;
            fill_descriptor(i, descriptor);

            descriptor.new_value = _last_value[i];
            descriptor.old_value = _last_value[i];

            send_message(i, descriptor, true);
        }
    }
}

void Analog::process_frame(const Frame& frame)
{
    if (is_frozen())
    {
        return;
    }

    for (size_t i = 0; i < Collection::size(GroupAnalogInputs); i++)
    {
        const auto physical_index = Remap::physical(i);

        if (physical_index >= frame.size())
        {
            continue;
        }

        process_reading(i, frame[physical_index]);
    }
}

void Analog::process_reading(size_t index, uint16_t value)
{
    if (is_frozen())
    {
        return;
    }

    // don't process component if it's not enabled
    if (!_database.read(database::Config::Section::Analog::Enable, index))
    {
        return;
    }

    Descriptor         analog_descriptor;
    Filter::Descriptor filter_descriptor;

    fill_descriptor(index, analog_descriptor);

    filter_descriptor.type         = analog_descriptor.type;
    filter_descriptor.value        = value;
    filter_descriptor.lower_offset = analog_descriptor.lower_offset;
    filter_descriptor.upper_offset = analog_descriptor.upper_offset;
    filter_descriptor.max_value    = analog_descriptor.max_value;

    if (!_filter.is_filtered(index, filter_descriptor))
    {
        return;
    }

    // assumption for now
    analog_descriptor.new_value = filter_descriptor.value;
    analog_descriptor.old_value = _last_value[index];

    bool send = false;

    switch (analog_descriptor.type)
    {
    case Type::PotentiometerControlChange:
    case Type::PotentiometerNote:
    case Type::Nrpn7Bit:
    case Type::Nrpn14Bit:
    case Type::PitchBend:
    case Type::ControlChange14Bit:
    {
        if (check_potentiometer_value(index, analog_descriptor))
        {
            send = true;
        }
    }
    break;

    case Type::Fsr:
    {
        if (check_fsr_value(index, analog_descriptor))
        {
            send = true;
        }
    }
    break;

    case Type::Button:
    {
        send = true;
    }
    break;

    default:
        break;
    }

    if (send)
    {
        send_message(index, analog_descriptor);
        _last_value[index] = analog_descriptor.new_value;
    }
}

bool Analog::check_potentiometer_value([[maybe_unused]] size_t index, Descriptor& descriptor)
{
    switch (descriptor.type)
    {
    case Type::Nrpn14Bit:
    case Type::PitchBend:
    case Type::ControlChange14Bit:
        break;

    default:
    {
        // use 7-bit MIDI ID and limits
        auto split_index        = util::Conversion::Split14Bit(descriptor.signal.index);
        descriptor.signal.index = split_index.low();

        auto split_lower_limit = util::Conversion::Split14Bit(descriptor.lower_limit);
        descriptor.lower_limit = split_lower_limit.low();

        auto split_upper_limit = util::Conversion::Split14Bit(descriptor.upper_limit);
        descriptor.upper_limit = split_upper_limit.low();
    }
    break;
    }

    if (descriptor.new_value > descriptor.max_value)
    {
        return false;
    }

    auto scale = [&](uint16_t value)
    {
        uint32_t scaled = 0;

        if (descriptor.lower_limit > descriptor.upper_limit)
        {
            scaled = zlibs::utils::misc::map_range(static_cast<uint32_t>(value),
                                                   static_cast<uint32_t>(0),
                                                   static_cast<uint32_t>(descriptor.max_value),
                                                   static_cast<uint32_t>(descriptor.upper_limit),
                                                   static_cast<uint32_t>(descriptor.lower_limit));

            if (!descriptor.inverted)
            {
                scaled = descriptor.upper_limit - (scaled - descriptor.lower_limit);
            }
        }
        else
        {
            scaled = zlibs::utils::misc::map_range(static_cast<uint32_t>(value),
                                                   static_cast<uint32_t>(0),
                                                   static_cast<uint32_t>(descriptor.max_value),
                                                   static_cast<uint32_t>(descriptor.lower_limit),
                                                   static_cast<uint32_t>(descriptor.upper_limit));

            if (descriptor.inverted)
            {
                scaled = descriptor.upper_limit - (scaled - descriptor.lower_limit);
            }
        }

        return scaled;
    };

    auto scaled = scale(descriptor.new_value);

    if (scaled == descriptor.old_value)
    {
        return false;
    }

    descriptor.new_value = scaled;

    return true;
}

bool Analog::check_fsr_value(size_t index, Descriptor& descriptor)
{
    // don't allow touchscreen components to be processed as Fsr
    if (index >= Collection::size(GroupAnalogInputs))
    {
        return false;
    }

    if (descriptor.new_value > 0)
    {
        if (!fsr_state(index))
        {
            // sensor is really pressed
            set_fsr_state(index, true);
            return true;
        }
    }
    else
    {
        if (fsr_state(index))
        {
            set_fsr_state(index, false);
            return true;
        }
    }

    return false;
}

void Analog::send_message([[maybe_unused]] size_t index, Descriptor& descriptor, bool ignore_freeze)
{
    if (is_frozen() && !ignore_freeze)
    {
        return;
    }

    descriptor.signal.value = descriptor.new_value;

    auto send_analog = [&]()
    {
        descriptor.signal.source = signaling::MidiSource::Analog;
        signaling::publish(descriptor.signal);
    };

    switch (descriptor.type)
    {
    case Type::PotentiometerControlChange:
    case Type::PotentiometerNote:
    case Type::PitchBend:
    case Type::Nrpn7Bit:
    case Type::Nrpn14Bit:
    {
        send_analog();
    }
    break;

    case Type::Fsr:
    {
        if (!descriptor.new_value)
        {
            descriptor.signal.message = midi::MessageType::NoteOff;
        }

        send_analog();
    }
    break;

    case Type::Button:
    {
        signaling::MidiSignal signal = {};
        signal.source                = signaling::MidiSource::AnalogButton;
        signal.component_index       = descriptor.signal.component_index;
        signal.value                 = descriptor.signal.value;

        signaling::publish(signal);
    }
    break;

    case Type::ControlChange14Bit:
    {
        if (descriptor.signal.index >= midi::CONTROL_CHANGE_14BIT_MAX_INDEX)
        {
            // not allowed
            return;
        }

        send_analog();
    }
    break;

    default:
        break;
    }
}

void Analog::reset(size_t index)
{
    set_fsr_state(index, false);
    _filter.reset(index);
    _last_value[index] = RESET_ANALOG_VALUE;
}

void Analog::set_fsr_state(size_t index, bool state)
{
    const auto location = io::common::bit_storage_location<STATE_STORAGE_DIVISOR>(index);

    zlibs::utils::misc::bit_write(_fsr_pressed[location.array_index], location.bit_index, state);
}

bool Analog::fsr_state(size_t index)
{
    const auto location = io::common::bit_storage_location<STATE_STORAGE_DIVISOR>(index);

    return zlibs::utils::misc::bit_read(_fsr_pressed[location.array_index], location.bit_index);
}

void Analog::fill_descriptor(size_t index, Descriptor& descriptor)
{
    descriptor.type                   = static_cast<Type>(_database.read(database::Config::Section::Analog::Type, index));
    descriptor.inverted               = _database.read(database::Config::Section::Analog::Invert, index);
    descriptor.lower_limit            = _database.read(database::Config::Section::Analog::LowerLimit, index);
    descriptor.upper_limit            = _database.read(database::Config::Section::Analog::UpperLimit, index);
    descriptor.lower_offset           = _database.read(database::Config::Section::Analog::LowerOffset, index);
    descriptor.upper_offset           = _database.read(database::Config::Section::Analog::UpperOffset, index);
    descriptor.signal.component_index = index;
    descriptor.signal.channel         = _database.read(database::Config::Section::Analog::Channel, index);
    descriptor.signal.index           = _database.read(database::Config::Section::Analog::MidiId, index);
    descriptor.signal.message         = INTERNAL_MSG_TO_MIDI_TYPE[static_cast<uint8_t>(descriptor.type)];

    switch (descriptor.type)
    {
    case Type::Nrpn14Bit:
    case Type::PitchBend:
    case Type::ControlChange14Bit:
    {
        descriptor.max_value = midi::MAX_VALUE_14BIT;
    }
    break;

    default:
    {
        descriptor.max_value = midi::MAX_VALUE_7BIT;
    }
    break;
    }
}

std::optional<uint8_t> Analog::sys_config_get(sys::Config::Section::Analog section, size_t index, uint16_t& value)
{
    uint32_t read_value = 0;

    auto result = _database.read(util::Conversion::sys_2_db_section(section), index, read_value)
                      ? sys::Config::Status::Ack
                      : sys::Config::Status::ErrorRead;

    switch (section)
    {
    case sys::Config::Section::Analog::Reserved1:
    case sys::Config::Section::Analog::Reserved2:
    case sys::Config::Section::Analog::Reserved3:
        return sys::Config::Status::ErrorNotSupported;

    default:
        break;
    }

    value = read_value;

    return result;
}

std::optional<uint8_t> Analog::sys_config_set(sys::Config::Section::Analog section, size_t index, uint16_t value)
{
    switch (section)
    {
    case sys::Config::Section::Analog::Reserved1:
    case sys::Config::Section::Analog::Reserved2:
    case sys::Config::Section::Analog::Reserved3:
        return sys::Config::Status::ErrorNotSupported;

    case sys::Config::Section::Analog::Type:
    {
        reset(index);
    }
    break;

    default:
        break;
    }

    return _database.update(util::Conversion::sys_2_db_section(section), index, value)
               ? sys::Config::Status::Ack
               : sys::Config::Status::ErrorWrite;
}

#endif
