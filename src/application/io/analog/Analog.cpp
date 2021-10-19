/*

Copyright 2015-2021 Igor Petrovic

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

#include "Analog.h"
#include "core/src/general/Helpers.h"

using namespace IO;

Analog::Analog(HWA&                     hwa,
               Filter&                  filter,
               Database&                database,
               Util::MessageDispatcher& dispatcher)
    : _hwa(hwa)
    , _filter(filter)
    , _database(database)
    , _dispatcher(dispatcher)
{
    _dispatcher.listen(Util::MessageDispatcher::messageSource_t::touchscreenAnalog,
                       Util::MessageDispatcher::listenType_t::forward,
                       [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                           size_t index = dispatchMessage.componentIndex + MAX_NUMBER_OF_ANALOG;
                           processReading(index, dispatchMessage.midiValue);
                       });
}

void Analog::update(bool forceResend)
{
    // check values
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        if (!forceResend)
        {
            uint16_t value;

            if (!_hwa.value(i, value))
                continue;

            processReading(i, value);
        }
        else
        {
            if (_database.read(Database::Section::analog_t::enable, i))
            {
                analogDescriptor_t descriptor;
                fillAnalogDescriptor(i, descriptor);
                descriptor.dispatchMessage.midiValue = _lastValue[i];
                sendMessage(i, descriptor);
            }
        }
    }
}

Analog::adcType_t Analog::adcType()
{
    return _filter.adcType();
}

void Analog::processReading(size_t index, uint16_t value)
{
    // don't process component if it's not enabled
    if (!_database.read(Database::Section::analog_t::enable, index))
        return;

    analogDescriptor_t descriptor;
    fillAnalogDescriptor(index, descriptor);

    if (!_filter.isFiltered(index, descriptor.type, value, value))
        return;

    descriptor.dispatchMessage.midiValue = value;

    bool send = false;

    if (descriptor.type != type_t::button)
    {
        switch (descriptor.type)
        {
        case type_t::potentiometerControlChange:
        case type_t::potentiometerNote:
        case type_t::nrpn7bit:
        case type_t::nrpn14bit:
        case type_t::pitchBend:
        case type_t::controlChange14bit:
        {
            if (checkPotentiometerValue(index, descriptor))
                send = true;
        }
        break;

        case type_t::fsr:
        {
            if (checkFSRvalue(index, descriptor))
                send = true;
        }
        break;

        default:
            break;
        }
    }
    else
    {
        send = true;
    }

    if (send)
    {
        sendMessage(index, descriptor);
        _lastValue[index] = descriptor.dispatchMessage.midiValue;
    }
}

bool Analog::checkPotentiometerValue(size_t index, analogDescriptor_t& descriptor)
{
    uint16_t maxLimit;

    if ((descriptor.type == type_t::nrpn14bit) || (descriptor.type == type_t::pitchBend) || (descriptor.type == type_t::controlChange14bit))
    {
        // 14-bit values are already read
        maxLimit = MIDI::MIDI_14_BIT_VALUE_MAX;
    }
    else
    {
        maxLimit = MIDI::MIDI_7_BIT_VALUE_MAX;

        MIDI::Split14bit split14bit;

        // use 7-bit MIDI ID and limits
        split14bit.split(descriptor.dispatchMessage.midiIndex);
        descriptor.dispatchMessage.midiIndex = split14bit.low();

        split14bit.split(descriptor.lowerLimit);
        descriptor.lowerLimit = split14bit.low();

        split14bit.split(descriptor.upperLimit);
        descriptor.upperLimit = split14bit.low();
    }

    if (descriptor.dispatchMessage.midiValue > maxLimit)
        return false;

    uint32_t scaledMIDIvalue;

    if (descriptor.lowerLimit > descriptor.upperLimit)
    {
        scaledMIDIvalue = core::misc::mapRange(static_cast<uint32_t>(descriptor.dispatchMessage.midiValue), static_cast<uint32_t>(0), static_cast<uint32_t>(maxLimit), static_cast<uint32_t>(descriptor.upperLimit), static_cast<uint32_t>(descriptor.lowerLimit));

        if (!descriptor.inverted)
            scaledMIDIvalue = descriptor.upperLimit - (scaledMIDIvalue - descriptor.lowerLimit);
    }
    else
    {
        scaledMIDIvalue = core::misc::mapRange(static_cast<uint32_t>(descriptor.dispatchMessage.midiValue), static_cast<uint32_t>(0), static_cast<uint32_t>(maxLimit), static_cast<uint32_t>(descriptor.lowerLimit), static_cast<uint32_t>(descriptor.upperLimit));

        if (descriptor.inverted)
            scaledMIDIvalue = descriptor.upperLimit - (scaledMIDIvalue - descriptor.lowerLimit);
    }

    if (scaledMIDIvalue == _lastValue[index])
        return false;

    descriptor.dispatchMessage.midiValue = scaledMIDIvalue;

    return true;
}

bool Analog::checkFSRvalue(size_t index, analogDescriptor_t& descriptor)
{
    // don't allow touchscreen components to be processed as FSR
    if (index >= MAX_NUMBER_OF_ANALOG)
        return false;

    if (descriptor.dispatchMessage.midiValue > 0)
    {
        if (!_fsrPressed[index])
        {
            // sensor is really pressed
            _fsrPressed[index] = true;
            return true;
        }
    }
    else
    {
        if (_fsrPressed[index])
        {
            _fsrPressed[index] = false;
            return true;
        }
    }

    return false;
}

void Analog::sendMessage(size_t index, analogDescriptor_t& descriptor)
{
    bool send    = true;
    bool forward = false;

    switch (descriptor.type)
    {
    case type_t::potentiometerControlChange:
    case type_t::potentiometerNote:
    case type_t::pitchBend:
        break;

    case type_t::fsr:
    {
        if (!_fsrPressed[index])
        {
            descriptor.dispatchMessage.midiValue = 0;
            descriptor.dispatchMessage.message   = MIDI::messageType_t::noteOff;
        }
    }
    break;

    case type_t::button:
    {
        forward = true;
    }
    break;

    case type_t::controlChange14bit:
    {
        if (descriptor.dispatchMessage.midiIndex >= 96)
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
        _dispatcher.notify(Util::MessageDispatcher::messageSource_t::analog,
                           descriptor.dispatchMessage,
                           forward ? Util::MessageDispatcher::listenType_t::forward : Util::MessageDispatcher::listenType_t::nonFwd);
    }
}

void Analog::debounceReset(size_t index)
{
    _fsrPressed[index] = false;
    _lastValue[index]  = 0xFFFF;
    _filter.reset(index);
}

void Analog::fillAnalogDescriptor(size_t index, analogDescriptor_t& descriptor)
{
    descriptor.type                           = static_cast<type_t>(_database.read(Database::Section::analog_t::type, index));
    descriptor.inverted                       = _database.read(Database::Section::analog_t::invert, index);
    descriptor.lowerLimit                     = _database.read(Database::Section::analog_t::lowerLimit, index);
    descriptor.upperLimit                     = _database.read(Database::Section::analog_t::upperLimit, index);
    descriptor.dispatchMessage.componentIndex = index;
    descriptor.dispatchMessage.midiChannel    = _database.read(Database::Section::analog_t::midiChannel, index);
    descriptor.dispatchMessage.midiIndex      = _database.read(Database::Section::analog_t::midiID, index);
    descriptor.dispatchMessage.message        = _internalMsgToMIDIType[static_cast<uint8_t>(descriptor.type)];
}