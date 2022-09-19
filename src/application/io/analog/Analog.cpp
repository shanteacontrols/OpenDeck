/*

Copyright 2015-2022 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#ifdef HW_SUPPORT_ADC

#include "Analog.h"
#include "sysex/src/SysExConf.h"
#include "system/Config.h"

#include "core/src/util/Util.h"
#include "util/conversion/Conversion.h"
#include "util/configurable/Configurable.h"

using namespace IO;

Analog::Analog(HWA&      hwa,
               Filter&   filter,
               Database& database)
    : _hwa(hwa)
    , _filter(filter)
    , _database(database)
{
    MIDIDispatcher.listen(Messaging::eventType_t::SYSTEM,
                          [this](const Messaging::event_t& event)
                          {
                              switch (event.systemMessage)
                              {
                              case Messaging::systemMessage_t::FORCE_IO_REFRESH:
                              {
                                  updateAll(true);
                              }
                              break;

                              default:
                                  break;
                              }
                          });

    ConfigHandler.registerConfig(
        System::Config::block_t::ANALOG,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sysConfigGet(static_cast<System::Config::Section::analog_t>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sysConfigSet(static_cast<System::Config::Section::analog_t>(section), index, value);
        });
}

bool Analog::init()
{
    for (size_t i = 0; i < Collection::size(); i++)
    {
        reset(i);
    }

    return true;
}

void Analog::updateSingle(size_t index, bool forceRefresh)
{
    if (index >= maxComponentUpdateIndex())
    {
        return;
    }

    if (!forceRefresh)
    {
        uint16_t value;

        if (!_hwa.value(index, value))
        {
            return;
        }

        processReading(index, value);
    }
    else
    {
        if (_database.read(Database::Config::Section::analog_t::ENABLE, index))
        {
            analogDescriptor_t descriptor;
            fillAnalogDescriptor(index, descriptor);
            descriptor.event.value = _filter.lastValue(index);
            sendMessage(index, descriptor);
        }
    }
}

void Analog::updateAll(bool forceRefresh)
{
    // check values
    for (size_t i = 0; i < Collection::size(GROUP_ANALOG_INPUTS); i++)
    {
        updateSingle(i, forceRefresh);
    }
}

size_t Analog::maxComponentUpdateIndex()
{
    return Collection::size(GROUP_ANALOG_INPUTS);
}

void Analog::processReading(size_t index, uint16_t value)
{
    // don't process component if it's not enabled
    if (!_database.read(Database::Config::Section::analog_t::ENABLE, index))
    {
        return;
    }

    analogDescriptor_t   analogDescriptor;
    Filter::descriptor_t filterDescriptor;

    fillAnalogDescriptor(index, analogDescriptor);

    filterDescriptor.type        = analogDescriptor.type;
    filterDescriptor.value       = value;
    filterDescriptor.lowerOffset = analogDescriptor.lowerOffset;
    filterDescriptor.upperOffset = analogDescriptor.upperOffset;
    filterDescriptor.maxValue    = analogDescriptor.maxValue;

    if (!_filter.isFiltered(index, filterDescriptor))
    {
        return;
    }

    analogDescriptor.event.value = filterDescriptor.value;

    bool send = false;

    switch (analogDescriptor.type)
    {
    case type_t::POTENTIOMETER_CONTROL_CHANGE:
    case type_t::POTENTIOMETER_NOTE:
    case type_t::NRPN_7BIT:
    case type_t::NRPN_14BIT:
    case type_t::PITCH_BEND:
    case type_t::CONTROL_CHANGE_14BIT:
    case type_t::DMX:
    {
        if (checkPotentiometerValue(index, analogDescriptor))
        {
            send = true;
        }
    }
    break;

    case type_t::FSR:
    {
        if (checkFSRvalue(index, analogDescriptor))
        {
            send = true;
        }
    }
    break;

    case type_t::BUTTON:
    {
        send = true;
    }
    break;

    default:
        break;
    }

    if (send)
    {
        sendMessage(index, analogDescriptor);
    }
}

bool Analog::checkPotentiometerValue(size_t index, analogDescriptor_t& descriptor)
{
    switch (descriptor.type)
    {
    case type_t::NRPN_14BIT:
    case type_t::PITCH_BEND:
    case type_t::CONTROL_CHANGE_14BIT:
        break;

    case type_t::DMX:
    {
        descriptor.event.index = 0;    // irrelevant
        descriptor.lowerLimit &= 0xFF;
        descriptor.upperLimit &= 0xFF;
    }
    break;

    default:
    {
        // use 7-bit MIDI ID and limits
        auto splitIndex        = Util::Conversion::Split14bit(descriptor.event.index);
        descriptor.event.index = splitIndex.low();

        auto splitLowerLimit  = Util::Conversion::Split14bit(descriptor.lowerLimit);
        descriptor.lowerLimit = splitLowerLimit.low();

        auto splitUpperLimit  = Util::Conversion::Split14bit(descriptor.upperLimit);
        descriptor.upperLimit = splitUpperLimit.low();
    }
    break;
    }

    if (descriptor.event.value > descriptor.maxValue)
    {
        return false;
    }

    auto scale = [&](uint16_t value)
    {
        uint32_t scaled = 0;

        if (descriptor.lowerLimit > descriptor.upperLimit)
        {
            scaled = core::util::MAP_RANGE(static_cast<uint32_t>(value),
                                           static_cast<uint32_t>(0),
                                           static_cast<uint32_t>(descriptor.maxValue),
                                           static_cast<uint32_t>(descriptor.upperLimit),
                                           static_cast<uint32_t>(descriptor.lowerLimit));

            if (!descriptor.inverted)
            {
                scaled = descriptor.upperLimit - (scaled - descriptor.lowerLimit);
            }
        }
        else
        {
            scaled = core::util::MAP_RANGE(static_cast<uint32_t>(value),
                                           static_cast<uint32_t>(0),
                                           static_cast<uint32_t>(descriptor.maxValue),
                                           static_cast<uint32_t>(descriptor.lowerLimit),
                                           static_cast<uint32_t>(descriptor.upperLimit));

            if (descriptor.inverted)
            {
                scaled = descriptor.upperLimit - (scaled - descriptor.lowerLimit);
            }
        }

        return scaled;
    };

    auto scaledNew = scale(descriptor.event.value);
    auto scaledOld = scale(_filter.lastValue(index));

    if (scaledNew == scaledOld)
    {
        return false;
    }

    descriptor.event.value = scaledNew;

    return true;
}

bool Analog::checkFSRvalue(size_t index, analogDescriptor_t& descriptor)
{
    // don't allow touchscreen components to be processed as FSR
    if (index >= Collection::size(GROUP_ANALOG_INPUTS))
    {
        return false;
    }

    if (descriptor.event.value > 0)
    {
        if (!fsrState(index))
        {
            // sensor is really pressed
            setFSRstate(index, true);
            return true;
        }
    }
    else
    {
        if (fsrState(index))
        {
            setFSRstate(index, false);
            return true;
        }
    }

    return false;
}

void Analog::sendMessage(size_t index, analogDescriptor_t& descriptor)
{
    bool send      = true;
    auto eventType = Messaging::eventType_t::ANALOG;

    switch (descriptor.type)
    {
    case type_t::POTENTIOMETER_CONTROL_CHANGE:
    case type_t::POTENTIOMETER_NOTE:
    case type_t::PITCH_BEND:
        break;

    case type_t::FSR:
    {
        if (!fsrState(index))
        {
            descriptor.event.value   = 0;
            descriptor.event.message = MIDI::messageType_t::NOTE_OFF;
        }
    }
    break;

    case type_t::BUTTON:
    {
        eventType = Messaging::eventType_t::ANALOG_BUTTON;
    }
    break;

    case type_t::DMX:
    {
        eventType = Messaging::eventType_t::DMX_ANALOG;
    }
    break;

    case type_t::CONTROL_CHANGE_14BIT:
    {
        if (descriptor.event.index >= 96)
        {
            // not allowed
            send = false;
            break;
        }
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
        MIDIDispatcher.notify(eventType, descriptor.event);
    }
}

void Analog::reset(size_t index)
{
    setFSRstate(index, false);
    _filter.reset(index);
}

void Analog::setFSRstate(size_t index, bool state)
{
    uint8_t arrayIndex  = index / 8;
    uint8_t analogIndex = index - 8 * arrayIndex;

    core::util::BIT_WRITE(_fsrPressed[arrayIndex], analogIndex, state);
}

bool Analog::fsrState(size_t index)
{
    uint8_t arrayIndex  = index / 8;
    uint8_t analogIndex = index - 8 * arrayIndex;

    return core::util::BIT_READ(_fsrPressed[arrayIndex], analogIndex);
}

void Analog::fillAnalogDescriptor(size_t index, analogDescriptor_t& descriptor)
{
    descriptor.type                 = static_cast<type_t>(_database.read(Database::Config::Section::analog_t::TYPE, index));
    descriptor.inverted             = _database.read(Database::Config::Section::analog_t::INVERT, index);
    descriptor.lowerLimit           = _database.read(Database::Config::Section::analog_t::LOWER_LIMIT, index);
    descriptor.upperLimit           = _database.read(Database::Config::Section::analog_t::UPPER_LIMIT, index);
    descriptor.lowerOffset          = _database.read(Database::Config::Section::analog_t::LOWER_OFFSET, index);
    descriptor.upperOffset          = _database.read(Database::Config::Section::analog_t::UPPER_OFFSET, index);
    descriptor.event.componentIndex = index;
    descriptor.event.channel        = _database.read(Database::Config::Section::analog_t::CHANNEL, index);
    descriptor.event.index          = _database.read(Database::Config::Section::analog_t::MIDI_ID, index);
    descriptor.event.message        = INTERNAL_MSG_TO_MIDI_TYPE[static_cast<uint8_t>(descriptor.type)];

    switch (descriptor.type)
    {
    case type_t::NRPN_14BIT:
    case type_t::PITCH_BEND:
    case type_t::CONTROL_CHANGE_14BIT:
    {
        descriptor.maxValue = MIDI::MIDI_14BIT_VALUE_MAX;
    }
    break;

    case type_t::DMX:
    {
        descriptor.maxValue = 255;
    }
    break;

    default:
    {
        descriptor.maxValue = MIDI::MIDI_7BIT_VALUE_MAX;
    }
    break;
    }
}

std::optional<uint8_t> Analog::sysConfigGet(System::Config::Section::analog_t section, size_t index, uint16_t& value)
{
    uint32_t readValue;

    auto result = _database.read(Util::Conversion::sys2DBsection(section), index, readValue)
                      ? System::Config::status_t::ACK
                      : System::Config::status_t::ERROR_READ;

    switch (section)
    {
    case System::Config::Section::analog_t::MIDI_ID_MSB:
    case System::Config::Section::analog_t::LOWER_LIMIT_MSB:
    case System::Config::Section::analog_t::UPPER_LIMIT_MSB:
        return System::Config::status_t::ERROR_NOT_SUPPORTED;

    default:
        break;
    }

    value = readValue;

    return result;
}

std::optional<uint8_t> Analog::sysConfigSet(System::Config::Section::analog_t section, size_t index, uint16_t value)
{
    switch (section)
    {
    case System::Config::Section::analog_t::MIDI_ID_MSB:
    case System::Config::Section::analog_t::LOWER_LIMIT_MSB:
    case System::Config::Section::analog_t::UPPER_LIMIT_MSB:
        return System::Config::status_t::ERROR_NOT_SUPPORTED;

    case System::Config::Section::analog_t::TYPE:
    {
        reset(index);
    }
    break;

    default:
        break;
    }

    return _database.update(Util::Conversion::sys2DBsection(section), index, value)
               ? System::Config::status_t::ACK
               : System::Config::status_t::ERROR_WRITE;
}

#endif