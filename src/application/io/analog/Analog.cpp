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

void Analog::update(bool forceResend)
{
    //check values
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
                analogDescriptor_t analogDescriptor;
                fillAnalogDescriptor(i, analogDescriptor);

                sendMessage(i, analogDescriptor, _lastValue[i]);
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
    //don't process component if it's not enabled
    if (!_database.read(Database::Section::analog_t::enable, index))
        return;

    analogDescriptor_t analogDescriptor;
    fillAnalogDescriptor(index, analogDescriptor);

    if (!_filter.isFiltered(index, analogDescriptor.type, value, value))
        return;

    bool send = false;

    if (analogDescriptor.type != type_t::button)
    {
        switch (analogDescriptor.type)
        {
        case type_t::potentiometerControlChange:
        case type_t::potentiometerNote:
        case type_t::nrpn7b:
        case type_t::nrpn14b:
        case type_t::pitchBend:
        case type_t::cc14bit:
        {
            if (checkPotentiometerValue(index, analogDescriptor, value))
                send = true;
        }
        break;

        case type_t::fsr:
        {
            if (checkFSRvalue(index, analogDescriptor, value))
                send = true;
        }
        break;

        default:
            break;
        }
    }
    else
    {
        if (_buttonHandler != nullptr)
            _buttonHandler(index, value);
    }

    if (send)
    {
        sendMessage(index, analogDescriptor, value);
        _lastValue[index] = value;
    }

    _cInfo.send(Database::block_t::analog, index);
}

bool Analog::checkPotentiometerValue(size_t index, analogDescriptor_t& descriptor, uint16_t& value)
{
    uint16_t maxLimit;

    if ((descriptor.type == type_t::nrpn14b) || (descriptor.type == type_t::pitchBend) || (descriptor.type == type_t::cc14bit))
    {
        //14-bit values are already read
        maxLimit = MIDI_14_BIT_VALUE_MAX;
    }
    else
    {
        maxLimit = MIDI_7_BIT_VALUE_MAX;

        MIDI::Split14bit split14bit;

        //use 7-bit MIDI ID and limits
        split14bit.split(descriptor.midiID);
        descriptor.midiID = split14bit.low();

        split14bit.split(descriptor.lowerLimit);
        descriptor.lowerLimit = split14bit.low();

        split14bit.split(descriptor.upperLimit);
        descriptor.upperLimit = split14bit.low();
    }

    if (value > maxLimit)
        return false;

    uint32_t scaledMIDIvalue;

    if (descriptor.lowerLimit > descriptor.upperLimit)
    {
        scaledMIDIvalue = core::misc::mapRange(static_cast<uint32_t>(value), static_cast<uint32_t>(0), static_cast<uint32_t>(maxLimit), static_cast<uint32_t>(descriptor.upperLimit), static_cast<uint32_t>(descriptor.lowerLimit));

        if (!descriptor.inverted)
            scaledMIDIvalue = descriptor.upperLimit - (scaledMIDIvalue - descriptor.lowerLimit);
    }
    else
    {
        scaledMIDIvalue = core::misc::mapRange(static_cast<uint32_t>(value), static_cast<uint32_t>(0), static_cast<uint32_t>(maxLimit), static_cast<uint32_t>(descriptor.lowerLimit), static_cast<uint32_t>(descriptor.upperLimit));

        if (descriptor.inverted)
            scaledMIDIvalue = descriptor.upperLimit - (scaledMIDIvalue - descriptor.lowerLimit);
    }

    if (scaledMIDIvalue == _lastValue[index])
        return false;

    value = scaledMIDIvalue;

    return true;
}

bool Analog::checkFSRvalue(size_t index, analogDescriptor_t& descriptor, uint16_t& value)
{
    //don't allow touchscreen components to be processed as FSR
    if (index >= MAX_NUMBER_OF_ANALOG)
        return false;

    if (value > 0)
    {
        if (!_fsrPressed[index])
        {
            //sensor is really pressed
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

void Analog::sendMessage(size_t index, analogDescriptor_t& descriptor, uint16_t value)
{
    switch (descriptor.type)
    {
    case type_t::potentiometerControlChange:
    case type_t::potentiometerNote:
    {
        if (descriptor.type == type_t::potentiometerControlChange)
        {
            _midi.sendControlChange(descriptor.midiID, value, descriptor.channel);
            _display.displayMIDIevent(Display::eventType_t::out, Display::event_t::controlChange, descriptor.midiID, value, descriptor.channel + 1);
        }
        else
        {
            _midi.sendNoteOn(descriptor.midiID, value, descriptor.channel);
            _display.displayMIDIevent(Display::eventType_t::out, Display::event_t::noteOn, descriptor.midiID, value, descriptor.channel + 1);
        }
    }
    break;

    case type_t::nrpn7b:
    case type_t::nrpn14b:
    case type_t::cc14bit:
    {
        MIDI::Split14bit split14bit;

        //when nrpn/cc14bit is used, MIDI ID is split into two messages
        //first message contains higher byte

        split14bit.split(descriptor.midiID);

        if (descriptor.type != type_t::cc14bit)
        {
            _midi.sendControlChange(99, split14bit.high(), descriptor.channel);
            _midi.sendControlChange(98, split14bit.low(), descriptor.channel);
        }

        if (descriptor.type == type_t::nrpn7b)
        {
            _midi.sendControlChange(6, value, descriptor.channel);
        }
        else
        {
            descriptor.midiID = split14bit.low();

            //send 14-bit value in another two messages
            //first message contains higher byte
            split14bit.split(value);

            if (descriptor.type == type_t::cc14bit)
            {
                if (descriptor.midiID >= 96)
                    break;    //not allowed

                _midi.sendControlChange(descriptor.midiID, split14bit.high(), descriptor.channel);
                _midi.sendControlChange(descriptor.midiID + 32, split14bit.low(), descriptor.channel);
            }
            else
            {
                _midi.sendControlChange(6, split14bit.high(), descriptor.channel);
                _midi.sendControlChange(38, split14bit.low(), descriptor.channel);
            }
        }

        _display.displayMIDIevent(Display::eventType_t::out, (descriptor.type == type_t::cc14bit) ? Display::event_t::controlChange : Display::event_t::nrpn, descriptor.midiID, value, descriptor.channel + 1);
    }
    break;

    case type_t::pitchBend:
    {
        _midi.sendPitchBend(value, descriptor.channel);
        _display.displayMIDIevent(Display::eventType_t::out, Display::event_t::pitchBend, descriptor.midiID, value, descriptor.channel + 1);
    }
    break;

    case type_t::fsr:
    {
        if (_fsrPressed[index])
        {
            _midi.sendNoteOn(descriptor.midiID, value, descriptor.channel);
            _display.displayMIDIevent(Display::eventType_t::out, Display::event_t::noteOn, descriptor.midiID, value, descriptor.channel + 1);
            _leds.midiToState(MIDI::messageType_t::noteOn, descriptor.midiID, value, descriptor.channel, LEDs::dataSource_t::internal);
        }
        else
        {
            _midi.sendNoteOff(descriptor.midiID, 0, descriptor.channel);
            _display.displayMIDIevent(Display::eventType_t::out, Display::event_t::noteOff, descriptor.midiID, value, descriptor.channel + 1);
            _leds.midiToState(MIDI::messageType_t::noteOff, descriptor.midiID, 0, descriptor.channel, LEDs::dataSource_t::internal);
        }
    }
    break;

    default:
        break;
    }
}

void Analog::debounceReset(size_t index)
{
    _fsrPressed[index] = false;
    _lastValue[index]  = 0xFFFF;
    _filter.reset(index);
}

void Analog::registerButtonHandler(buttonHandler_t handler)
{
    _buttonHandler = std::move(handler);
}

void Analog::fillAnalogDescriptor(size_t index, analogDescriptor_t& analogDescriptor)
{
    analogDescriptor.type       = static_cast<type_t>(_database.read(Database::Section::analog_t::type, index));
    analogDescriptor.lowerLimit = _database.read(Database::Section::analog_t::lowerLimit, index);
    analogDescriptor.upperLimit = _database.read(Database::Section::analog_t::upperLimit, index);
    analogDescriptor.midiID     = _database.read(Database::Section::analog_t::midiID, index);
    analogDescriptor.channel    = _database.read(Database::Section::analog_t::midiChannel, index);
    analogDescriptor.inverted   = _database.read(Database::Section::analog_t::invert, index);
}