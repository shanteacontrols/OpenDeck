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

#include "Encoders.h"

#ifdef ENCODERS_SUPPORTED

#include "core/Timing.h"
#include "core/util/Util.h"
#include "application/util/conversion/Conversion.h"
#include "application/util/configurable/Configurable.h"

using namespace io;

Encoders::Encoders(HWA&      hwa,
                   Filter&   filter,
                   Database& database,
                   uint32_t  timeDiffTimeout)
    : _hwa(hwa)
    , _filter(filter)
    , _database(database)
    , TIME_DIFF_READOUT(timeDiffTimeout)
{
    MIDIDispatcher.listen(messaging::eventType_t::MIDI_IN,
                          [this](const messaging::event_t& event)
                          {
                              const uint8_t GLOBAL_CHANNEL = _database.read(database::Config::Section::global_t::MIDI_SETTINGS, MIDI::setting_t::GLOBAL_CHANNEL);
                              const uint8_t CHANNEL        = _database.read(database::Config::Section::global_t::MIDI_SETTINGS,
                                                                     MIDI::setting_t::USE_GLOBAL_CHANNEL)
                                                                 ? GLOBAL_CHANNEL
                                                                 : event.channel;

                              const bool USE_OMNI = CHANNEL == MIDI::MIDI_CHANNEL_OMNI ? true : false;

                              switch (event.message)
                              {
                              case MIDI::messageType_t::CONTROL_CHANGE:
                              {
                                  for (size_t i = 0; i < Collection::SIZE(); i++)
                                  {
                                      if (!_database.read(database::Config::Section::encoder_t::REMOTE_SYNC, i))
                                      {
                                          continue;
                                      }

                                      if (_database.read(database::Config::Section::encoder_t::MODE, i) != static_cast<int32_t>(io::Encoders::type_t::CONTROL_CHANGE))
                                      {
                                          continue;
                                      }

                                      if (!USE_OMNI)
                                      {
                                          if (_database.read(database::Config::Section::encoder_t::CHANNEL, i) != CHANNEL)
                                          {
                                              continue;
                                          }
                                      }

                                      if (_database.read(database::Config::Section::encoder_t::MIDI_ID, i) != event.index)
                                      {
                                          continue;
                                      }

                                      setValue(i, event.value);
                                  }
                              }
                              break;

                              default:
                                  break;
                              }
                          });

    ConfigHandler.registerConfig(
        sys::Config::block_t::ENCODERS,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sysConfigGet(static_cast<sys::Config::Section::encoder_t>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sysConfigSet(static_cast<sys::Config::Section::encoder_t>(section), index, value);
        });
}

bool Encoders::init()
{
    for (size_t i = 0; i < Collection::SIZE(); i++)
    {
        reset(i);
    }

    return true;
}

void Encoders::updateSingle(size_t index, bool forceRefresh)
{
    if (index >= maxComponentUpdateIndex())
    {
        return;
    }

    if (!_database.read(database::Config::Section::encoder_t::ENABLE, index))
    {
        return;
    }

    uint8_t  numberOfReadings = 0;
    uint32_t states           = 0;

    if (!_hwa.state(index, numberOfReadings, states))
    {
        return;
    }

    uint32_t currentTime = core::timing::ms();

    for (uint8_t reading = 0; reading < numberOfReadings; reading++)
    {
        // take into account that there is a 1ms difference between readouts
        // when processing, newest sample has index 0
        // start from oldest reading which is in upper bits
        uint8_t  processIndex = numberOfReadings - 1 - reading;
        uint32_t sampleTime   = currentTime - (TIME_DIFF_READOUT * processIndex);

        // there are two readings per encoder
        uint8_t pairState = states >> (processIndex * 2);
        pairState &= 0x03;

        // when processing, newest sample has index 0
        processReading(index, pairState, sampleTime);
    }
}

void Encoders::updateAll(bool forceRefresh)
{
    for (size_t i = 0; i < Collection::SIZE(); i++)
    {
        updateSingle(i, forceRefresh);
    }
}

size_t Encoders::maxComponentUpdateIndex()
{
    return Collection::SIZE();
}

void Encoders::processReading(size_t index, uint8_t pairValue, uint32_t sampleTime)
{
    position_t encoderState = read(index, pairValue);

    if (_filter.isFiltered(index, encoderState, encoderState, sampleTime))
    {
        if (encoderState != position_t::STOPPED)
        {
            if (_database.read(database::Config::Section::encoder_t::INVERT, index))
            {
                if (encoderState == position_t::CCW)
                {
                    encoderState = position_t::CW;
                }
                else
                {
                    encoderState = position_t::CCW;
                }
            }

            uint8_t encAcceleration = _database.read(database::Config::Section::encoder_t::ACCELERATION, index);

            if (encAcceleration)
            {
                // when time difference between two movements is smaller than ENCODERS_SPEED_TIMEOUT,
                // start accelerating
                if ((sampleTime - _filter.lastMovementTime(index)) < ENCODERS_SPEED_TIMEOUT)
                {
                    _encoderSpeed[index] = core::util::CONSTRAIN(static_cast<uint8_t>(_encoderSpeed[index] + ENCODER_SPEED_CHANGE[encAcceleration]),
                                                                 static_cast<uint8_t>(0),
                                                                 ENCODER_ACC_STEP_INC[encAcceleration]);
                }
                else
                {
                    _encoderSpeed[index] = 0;
                }
            }

            encoderDescriptor_t descriptor;
            fillEncoderDescriptor(index, descriptor);

            sendMessage(index, encoderState, descriptor);
        }
    }
}

void Encoders::sendMessage(size_t index, position_t encoderState, encoderDescriptor_t& descriptor)
{
    auto    eventType = messaging::eventType_t::ENCODER;
    bool    send      = true;
    uint8_t steps     = (_encoderSpeed[index] > 0) ? _encoderSpeed[index] : 1;

    switch (descriptor.type)
    {
    case type_t::CONTROL_CHANGE_7FH01H:
    {
        descriptor.event.value = VAL_CONTROL_CHANGE_7FH01H[static_cast<uint8_t>(encoderState)];
    }
    break;

    case type_t::CONTROL_CHANGE_3FH41H:
    {
        descriptor.event.value = VAL_CONTROL_CHANGE_3FH41H[static_cast<uint8_t>(encoderState)];
    }
    break;

    case type_t::CONTROL_CHANGE_41H01H:
    {
        descriptor.event.value = VAL_CONTROL_CHANGE_41H01H[static_cast<uint8_t>(encoderState)];
    }
    break;

    case type_t::PROGRAM_CHANGE:
    {
        if (encoderState == position_t::CCW)
        {
            if (!MIDIProgram.incrementProgram(descriptor.event.channel, 1))
            {
                send = false;    // edge value reached, nothing more to send
            }
        }
        else
        {
            if (!MIDIProgram.decrementProgram(descriptor.event.channel, 1))
            {
                send = false;    // edge value reached, nothing more to send
            }
        }

        descriptor.event.value = MIDIProgram.program(descriptor.event.channel);
    }
    break;

    case type_t::CONTROL_CHANGE:
    case type_t::PITCH_BEND:
    case type_t::NRPN_7BIT:
    case type_t::NRPN_14BIT:
    case type_t::CONTROL_CHANGE_14BIT:
    {
        const bool USE_14BIT =
            ((descriptor.type == type_t::PITCH_BEND) || (descriptor.type == type_t::NRPN_14BIT) || (descriptor.type == type_t::CONTROL_CHANGE_14BIT));

        if (USE_14BIT && (steps > 1))
        {
            steps <<= 2;
        }

        if (encoderState == position_t::CCW)
        {
            // ValueIncDecMIDI7Bit is used, but any type can be used when decrementing since the limit is 0 for all of them
            _value[index] = ValueIncDecMIDI7Bit::decrement(_value[index],
                                                           steps,
                                                           ValueIncDecMIDI7Bit::type_t::EDGE);
        }
        else
        {
            switch (descriptor.type)
            {
            case type_t::CONTROL_CHANGE:
            case type_t::NRPN_7BIT:
            {
                _value[index] = ValueIncDecMIDI7Bit::increment(_value[index],
                                                               steps,
                                                               ValueIncDecMIDI7Bit::type_t::EDGE);
            }
            break;

            case type_t::PITCH_BEND:
            case type_t::NRPN_14BIT:
            case type_t::CONTROL_CHANGE_14BIT:
            {
                _value[index] = ValueIncDecMIDI14Bit::increment(_value[index],
                                                                steps,
                                                                ValueIncDecMIDI14Bit::type_t::EDGE);
            }
            break;

            default:
                break;
            }
        }

        if (descriptor.type == type_t::CONTROL_CHANGE_14BIT)
        {
            if (descriptor.event.index >= 96)
            {
                // not allowed
                send = false;
            }
        }

        descriptor.event.value = _value[index];
    }
    break;

    case type_t::PRESET_CHANGE:
    {
        eventType                      = messaging::eventType_t::SYSTEM;
        descriptor.event.systemMessage = (encoderState == position_t::CW)
                                             ? messaging::systemMessage_t::PRESET_CHANGE_INC_REQ
                                             : messaging::systemMessage_t::PRESET_CHANGE_DEC_REQ;
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

/// Sets the MIDI value of specified encoder to default.
void Encoders::reset(size_t index)
{
    if (_database.read(database::Config::Section::encoder_t::MODE, index) == static_cast<int32_t>(type_t::PITCH_BEND))
    {
        _value[index] = 8192;
    }
    else
    {
        _value[index] = 0;
    }

    _filter.reset(index);
    _encoderSpeed[index]  = 0;
    _encoderData[index]   = 0;
    _encoderPulses[index] = 0;
}

void Encoders::setValue(size_t index, uint16_t value)
{
    _value[index] = value;
}

/// Checks state of requested encoder.
/// param [in]: index           Encoder which is being checked.
/// param [in]: pairState       A and B signal readings from encoder placed into bits 0 and 1.
/// returns: Encoder direction. See position_t.
Encoders::position_t Encoders::read(size_t index, uint8_t pairState)
{
    position_t retVal = position_t::STOPPED;
    pairState &= 0x03;

    // add new data

    bool process = true;

    // only process the data from encoder if there is a previous reading stored
    if (!core::util::BIT_READ(_encoderData[index], 7))
    {
        process = false;
    }

    _encoderData[index] <<= 2;
    _encoderData[index] |= pairState;
    _encoderData[index] |= 0x80;

    if (!process)
    {
        return retVal;
    }

    _encoderPulses[index] += ENCODER_LOOK_UP_TABLE[_encoderData[index] & 0x0F];

    if (abs(_encoderPulses[index]) >= static_cast<int32_t>(_database.read(database::Config::Section::encoder_t::PULSES_PER_STEP, index)))
    {
        retVal = (_encoderPulses[index] > 0) ? position_t::CCW : position_t::CW;
        // reset count
        _encoderPulses[index] = 0;
    }

    return retVal;
}

void Encoders::fillEncoderDescriptor(size_t index, encoderDescriptor_t& descriptor)
{
    descriptor.type                 = static_cast<type_t>(_database.read(database::Config::Section::encoder_t::MODE, index));
    descriptor.event.componentIndex = index;
    descriptor.event.channel        = _database.read(database::Config::Section::encoder_t::CHANNEL, index);
    descriptor.event.index          = _database.read(database::Config::Section::encoder_t::MIDI_ID, index);
    descriptor.event.message        = INTERNAL_MSG_TO_MIDI_TYPE[static_cast<uint8_t>(descriptor.type)];
}

std::optional<uint8_t> Encoders::sysConfigGet(sys::Config::Section::encoder_t section, size_t index, uint16_t& value)
{
    uint32_t readValue;

    auto result = _database.read(util::Conversion::SYS_2_DB_SECTION(section), index, readValue)
                      ? sys::Config::status_t::ACK
                      : sys::Config::status_t::ERROR_READ;

    if (result == sys::Config::status_t::ACK)
    {
        if (section == sys::Config::Section::encoder_t::MIDI_ID_MSB)
        {
            return sys::Config::status_t::ERROR_NOT_SUPPORTED;
        }
    }

    value = readValue;

    return result;
}

std::optional<uint8_t> Encoders::sysConfigSet(sys::Config::Section::encoder_t section, size_t index, uint16_t value)
{
    switch (section)
    {
    case sys::Config::Section::encoder_t::MIDI_ID_MSB:
        return sys::Config::status_t::ERROR_NOT_SUPPORTED;

    default:
        break;
    }

    auto result = _database.update(util::Conversion::SYS_2_DB_SECTION(section), index, value)
                      ? sys::Config::status_t::ACK
                      : sys::Config::status_t::ERROR_WRITE;

    if (result == sys::Config::status_t::ACK)
    {
        reset(index);
    }

    return result;
}

#endif