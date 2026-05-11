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
}    // namespace

Analog::Analog(Hwa&      hwa,
               Filter&   filter,
               Mapper&   mapper,
               Database& database)
    : _hwa(hwa)
    , _filter(filter)
    , _mapper(mapper)
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

                      std::optional<Frame> frame;

                      {
                          const zlibs::utils::misc::LockGuard lock(_hwa_mutex);
                          frame = _hwa.read();
                      }

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

    signaling::subscribe<signaling::SystemSignal>(
        [this](const signaling::SystemSignal& signal)
        {
            if (signal.system_event != signaling::SystemEvent::PresetChanged)
            {
                return;
            }

            const zlibs::utils::misc::LockGuard lock(_hwa_mutex);
            update_scan_mask();
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

    {
        const zlibs::utils::misc::LockGuard lock(_hwa_mutex);
        update_scan_mask();
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
            Filter::Descriptor filter_descriptor = {};
            fill_filter_descriptor(i, filter_descriptor);

            if (const auto result = _mapper.last_result(i); result.has_value())
            {
                if (filter_descriptor.type == Type::Button)
                {
                    publish_button(result.value(), true);
                }
                else
                {
                    publish_result(result.value(), true);
                }
            }
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

    Filter::Descriptor filter_descriptor = {};
    fill_filter_descriptor(index, filter_descriptor);

    filter_descriptor.value = value;
    if (!_filter.is_filtered(index, filter_descriptor))
    {
        return;
    }

    const auto position = filter_descriptor.value;
    const auto result   = _mapper.result(index, position);

    switch (filter_descriptor.type)
    {
    case Type::PotentiometerControlChange:
    case Type::PotentiometerNote:
    case Type::Fsr:
    case Type::Nrpn7Bit:
    case Type::Nrpn14Bit:
    case Type::PitchBend:
    case Type::ControlChange14Bit:
    {
        if (result.has_value())
        {
            publish_result(result.value());
        }
    }
    break;

    case Type::Button:
    {
        if (result.has_value())
        {
            publish_button(result.value());
        }
    }
    break;

    default:
        break;
    }
}

void Analog::update_scan_mask()
{
    ScanMask mask = {};

    for (size_t i = 0; i < Collection::size(GroupAnalogInputs); i++)
    {
        if (!_database.read(database::Config::Section::Analog::Enable, i))
        {
            continue;
        }

        const auto physical_index = Remap::physical(i);

        if (physical_index < mask.size())
        {
            mask[physical_index] = true;
        }
    }

    _hwa.set_scan_mask(mask);
}

void Analog::publish_result(const Mapper::Result& result, bool ignore_freeze)
{
    if (is_frozen() && !ignore_freeze)
    {
        return;
    }

    if (result.osc.has_value())
    {
        signaling::publish(*result.osc);
    }

    if (result.midi.has_value())
    {
        signaling::publish(*result.midi);
    }
}

void Analog::reset(size_t index)
{
    _filter.reset(index);
    _mapper.reset(index);
}

void Analog::fill_filter_descriptor(size_t index, Filter::Descriptor& filter_descriptor)
{
    filter_descriptor.type         = static_cast<Type>(_database.read(database::Config::Section::Analog::Type, index));
    filter_descriptor.lower_offset = _database.read(database::Config::Section::Analog::LowerOffset, index);
    filter_descriptor.upper_offset = _database.read(database::Config::Section::Analog::UpperOffset, index);
}

void Analog::publish_button(const Mapper::Result& result, bool ignore_freeze)
{
    if (is_frozen() && !ignore_freeze)
    {
        return;
    }

    if (!result.midi.has_value())
    {
        return;
    }

    signaling::MidiIoSignal signal = {};
    signal.source                  = signaling::IoEventSource::AnalogButton;
    signal.component_index         = result.midi->component_index;
    signal.value                   = result.midi->value;

    signaling::publish(signaling::OscIoSignal{
        .source          = signaling::IoEventSource::AnalogButton,
        .component_index = signal.component_index,
        .int32_value     = static_cast<int32_t>(signal.value),
        .direction       = signaling::SignalDirection::Out,
    });

    signaling::publish(signal);
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

    case sys::Config::Section::Analog::Enable:
    case sys::Config::Section::Analog::Type:
    {
        reset(index);
    }
    break;

    default:
        break;
    }

    if (!_database.update(util::Conversion::sys_2_db_section(section), index, value))
    {
        return sys::Config::Status::ErrorWrite;
    }

    if (section == sys::Config::Section::Analog::Enable)
    {
        const zlibs::utils::misc::LockGuard lock(_hwa_mutex);
        update_scan_mask();
    }

    return sys::Config::Status::Ack;
}

#endif
