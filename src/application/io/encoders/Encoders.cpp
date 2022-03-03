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

#include "core/src/general/Timing.h"
#include "core/src/general/Helpers.h"
#include "util/conversion/Conversion.h"
#include "util/configurable/Configurable.h"

using namespace IO;

Encoders::Encoders(HWA&      hwa,
                   Filter&   filter,
                   Database& database,
                   uint32_t  timeDiffTimeout)
    : _hwa(hwa)
    , _filter(filter)
    , _database(database)
    , TIME_DIFF_READOUT(timeDiffTimeout)
{
    MIDIDispatcher.listen(Messaging::eventSource_t::midiIn,
                          Messaging::listenType_t::nonFwd,
                          [this](const Messaging::event_t& event) {
                              switch (event.message)
                              {
                              case MIDI::messageType_t::controlChange:
                              {
                                  for (size_t i = 0; i < Collection::size(); i++)
                                  {
                                      if (!_database.read(Database::Section::encoder_t::remoteSync, i))
                                      {
                                          continue;
                                      }

                                      if (_database.read(Database::Section::encoder_t::mode, i) != static_cast<int32_t>(IO::Encoders::type_t::controlChange))
                                      {
                                          continue;
                                      }

                                      if (_database.read(Database::Section::encoder_t::midiChannel, i) != event.midiChannel)
                                      {
                                          continue;
                                      }

                                      if (_database.read(Database::Section::encoder_t::midiID, i) != event.midiIndex)
                                      {
                                          continue;
                                      }

                                      setValue(i, event.midiValue);
                                  }
                              }
                              break;

                              default:
                                  break;
                              }
                          });

    ConfigHandler.registerConfig(
        System::Config::block_t::encoders,
        // read
        [this](uint8_t section, size_t index, uint16_t& value) {
            return sysConfigGet(static_cast<System::Config::Section::encoder_t>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value) {
            return sysConfigSet(static_cast<System::Config::Section::encoder_t>(section), index, value);
        });
}

bool Encoders::init()
{
    for (size_t i = 0; i < Collection::size(); i++)
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

    if (!_database.read(Database::Section::encoder_t::enable, index))
    {
        return;
    }

    uint8_t  numberOfReadings = 0;
    uint32_t states           = 0;

    if (!_hwa.state(index, numberOfReadings, states))
    {
        return;
    }

    uint32_t currentTime = core::timing::currentRunTimeMs();

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
    for (size_t i = 0; i < Collection::size(); i++)
    {
        updateSingle(i, forceRefresh);
    }
}

size_t Encoders::maxComponentUpdateIndex()
{
    return Collection::size();
}

void Encoders::processReading(size_t index, uint8_t pairValue, uint32_t sampleTime)
{
    position_t encoderState = read(index, pairValue);

    if (_filter.isFiltered(index, encoderState, encoderState, sampleTime))
    {
        if (encoderState != position_t::stopped)
        {
            if (_database.read(Database::Section::encoder_t::invert, index))
            {
                if (encoderState == position_t::ccw)
                {
                    encoderState = position_t::cw;
                }
                else
                {
                    encoderState = position_t::ccw;
                }
            }

            uint8_t encAcceleration = _database.read(Database::Section::encoder_t::acceleration, index);

            if (encAcceleration)
            {
                // when time difference between two movements is smaller than ENCODERS_SPEED_TIMEOUT,
                // start accelerating
                if ((sampleTime - _filter.lastMovementTime(index)) < ENCODERS_SPEED_TIMEOUT)
                {
                    _encoderSpeed[index] = CONSTRAIN(_encoderSpeed[index] + _encoderSpeedChange[encAcceleration], 0, _encoderMaxAccSpeed[encAcceleration]);
                }
                else
                {
                    _encoderSpeed[index] = 0;
                }
            }

            encoderDescriptor_t descriptor;
            fillEncoderDescriptor(index, descriptor);

            bool    send  = true;
            uint8_t steps = (_encoderSpeed[index] > 0) ? _encoderSpeed[index] : 1;

            switch (descriptor.type)
            {
            case type_t::controlChange7Fh01h:
            case type_t::controlChange3Fh41h:
            {
                descriptor.event.midiValue = _encValue[static_cast<uint8_t>(descriptor.type)][static_cast<uint8_t>(encoderState)];
            }
            break;

            case type_t::programChange:
            {
                if (encoderState == position_t::ccw)
                {
                    if (!Common::pcIncrement(descriptor.event.midiChannel))
                    {
                        send = false;    // edge value reached, nothing more to send
                    }
                }
                else
                {
                    if (!Common::pcDecrement(descriptor.event.midiChannel))
                    {
                        send = false;    // edge value reached, nothing more to send
                    }
                }

                descriptor.event.midiValue = Common::program(descriptor.event.midiChannel);
            }
            break;

            case type_t::controlChange:
            case type_t::pitchBend:
            case type_t::nrpn7bit:
            case type_t::nrpn14bit:
            case type_t::controlChange14bit:
            {
                const bool use14bit =
                    ((descriptor.type == type_t::pitchBend) || (descriptor.type == type_t::nrpn14bit) || (descriptor.type == type_t::controlChange14bit));

                if (use14bit && (steps > 1))
                {
                    steps <<= 2;
                }

                if (encoderState == position_t::ccw)
                {
                    _midiValue[index] -= steps;

                    if (_midiValue[index] < 0)
                    {
                        _midiValue[index] = 0;
                    }
                }
                else
                {
                    int16_t limit = use14bit ? 16383 : 127;

                    _midiValue[index] += steps;

                    if (_midiValue[index] > limit)
                    {
                        _midiValue[index] = limit;
                    }
                }

                descriptor.event.midiValue = _midiValue[index];
            }
            break;

            case type_t::presetChange:
            {
                send           = false;
                uint8_t preset = _database.getPreset();
                preset += (encoderState == position_t::cw) ? 1 : -1;

                _database.setPreset(preset);
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
                sendMessage(index, descriptor);
            }
        }
    }
}

void Encoders::sendMessage(size_t index, encoderDescriptor_t& descriptor)
{
    bool send = true;

    switch (descriptor.type)
    {
    case type_t::controlChange7Fh01h:
    case type_t::controlChange3Fh41h:
    case type_t::programChange:
    case type_t::controlChange:
    case type_t::pitchBend:
    case type_t::nrpn7bit:
    case type_t::nrpn14bit:
        break;

    case type_t::controlChange14bit:
    {
        if (descriptor.event.midiIndex >= 96)
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
        MIDIDispatcher.notify(Messaging::eventSource_t::encoders,
                              descriptor.event,
                              Messaging::listenType_t::nonFwd);
    }
}

/// Sets the MIDI value of specified encoder to default.
void Encoders::reset(size_t index)
{
    if (_database.read(Database::Section::encoder_t::mode, index) == static_cast<int32_t>(type_t::pitchBend))
    {
        _midiValue[index] = 8192;
    }
    else
    {
        _midiValue[index] = 0;
    }

    _filter.reset(index);
    _encoderSpeed[index]  = 0;
    _encoderData[index]   = 0;
    _encoderPulses[index] = 0;
}

void Encoders::setValue(size_t index, uint16_t value)
{
    _midiValue[index] = value;
}

/// Checks state of requested encoder.
/// param [in]: index           Encoder which is being checked.
/// param [in]: pairState       A and B signal readings from encoder placed into bits 0 and 1.
/// returns: Encoder direction. See position_t.
Encoders::position_t Encoders::read(size_t index, uint8_t pairState)
{
    position_t returnValue = position_t::stopped;
    pairState &= 0x03;

    // add new data

    bool process = true;

    // only process the data from encoder if there is a previous reading stored
    if (!BIT_READ(_encoderData[index], 7))
    {
        process = false;
    }

    _encoderData[index] <<= 2;
    _encoderData[index] |= pairState;
    _encoderData[index] |= 0x80;

    if (!process)
    {
        return returnValue;
    }

    _encoderPulses[index] += _encoderLookUpTable[_encoderData[index] & 0x0F];

    if (abs(_encoderPulses[index]) >= _database.read(Database::Section::encoder_t::pulsesPerStep, index))
    {
        returnValue = (_encoderPulses[index] > 0) ? position_t::ccw : position_t::cw;
        // reset count
        _encoderPulses[index] = 0;
    }

    return returnValue;
}

void Encoders::fillEncoderDescriptor(size_t index, encoderDescriptor_t& descriptor)
{
    descriptor.type          = static_cast<type_t>(_database.read(Database::Section::encoder_t::mode, index));
    descriptor.pulsesPerStep = _database.read(Database::Section::encoder_t::pulsesPerStep, index);

    descriptor.event.componentIndex = index;
    descriptor.event.midiChannel    = _database.read(Database::Section::encoder_t::midiChannel, index);
    descriptor.event.midiIndex      = _database.read(Database::Section::encoder_t::midiID, index);
    descriptor.event.message        = _internalMsgToMIDIType[static_cast<uint8_t>(descriptor.type)];
}

std::optional<uint8_t> Encoders::sysConfigGet(System::Config::Section::encoder_t section, size_t index, uint16_t& value)
{
    int32_t readValue;
    auto    result = _database.read(Util::Conversion::sys2DBsection(section), index, readValue) ? System::Config::status_t::ack : System::Config::status_t::errorRead;

    if (result == System::Config::status_t::ack)
    {
        if (section == System::Config::Section::encoder_t::midiID_MSB)
        {
            return System::Config::status_t::errorNotSupported;
        }
    }

    value = readValue;

    return result;
}

std::optional<uint8_t> Encoders::sysConfigSet(System::Config::Section::encoder_t section, size_t index, uint16_t value)
{
    switch (section)
    {
    case System::Config::Section::encoder_t::midiID_MSB:
        return System::Config::status_t::errorNotSupported;

    default:
        break;
    }

    auto result = _database.update(Util::Conversion::sys2DBsection(section), index, value) ? System::Config::status_t::ack : System::Config::status_t::errorWrite;

    if (result == System::Config::status_t::ack)
    {
        reset(index);
    }

    return result;
}

#endif