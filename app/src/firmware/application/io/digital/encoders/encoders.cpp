/*

Copyright Igor Petrovic

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

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ENCODERS

#include "encoders.h"
#include "application/util/conversion/conversion.h"
#include "application/util/configurable/configurable.h"
#include "application/global/bpm.h"

#include "zlibs/utils/misc/bit.h"
#include "zlibs/utils/misc/numeric.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

using namespace io::encoders;
using namespace protocol;

namespace
{
    LOG_MODULE_REGISTER(encoders, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

Encoders::Encoders(Hwa&      hwa,
                   Filter&   filter,
                   Database& database,
                   uint32_t  timeDiffTimeout)
    : _hwa(hwa)
    , _filter(filter)
    , _database(database)
    , TIME_DIFF_READOUT(timeDiffTimeout)
{
    messaging::subscribe<messaging::UmpSignal>(
        [this](const messaging::UmpSignal& event)
        {
            if (event.direction != messaging::MidiDirection::In)
            {
                return;
            }

            const auto message = midi::decode_message(event.packet);

            const uint8_t GLOBAL_CHANNEL = _database.read(database::Config::Section::global_t::MIDI_SETTINGS, midi::setting_t::GLOBAL_CHANNEL);
            const uint8_t CHANNEL        = _database.read(database::Config::Section::global_t::MIDI_SETTINGS,
                                                          midi::setting_t::USE_GLOBAL_CHANNEL)
                                               ? GLOBAL_CHANNEL
                                               : message.channel;

            const bool USE_OMNI = CHANNEL == midi::OMNI_CHANNEL ? true : false;

            switch (message.type)
            {
            case midi::messageType_t::CONTROL_CHANGE:
            {
                for (size_t i = 0; i < Collection::SIZE(); i++)
                {
                    if (!_database.read(database::Config::Section::encoder_t::REMOTE_SYNC, i))
                    {
                        continue;
                    }

                    if (_database.read(database::Config::Section::encoder_t::MODE, i) != static_cast<int32_t>(type_t::CONTROL_CHANGE))
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

                    if (_database.read(database::Config::Section::encoder_t::MIDI_ID_1, i) != message.data1)
                    {
                        continue;
                    }

                    setValue(i, message.data2);
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

    const auto state = _hwa.state(index);

    if (!state.has_value())
    {
        return;
    }

    processReading(index, state.value(), k_uptime_get_32());
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
    auto position = read(index, pairValue);

    if (_filter.isFiltered(index, position, position, sampleTime))
    {
        if (position != position_t::STOPPED)
        {
            if (_database.read(database::Config::Section::encoder_t::INVERT, index))
            {
                if (position == position_t::CCW)
                {
                    position = position_t::CW;
                }
                else
                {
                    position = position_t::CCW;
                }
            }

            uint8_t encAcceleration = _database.read(database::Config::Section::encoder_t::ACCELERATION, index);

            if (encAcceleration)
            {
                // when time difference between two movements is smaller than ENCODERS_SPEED_TIMEOUT,
                // start accelerating
                if ((sampleTime - _filter.lastMovementTime(index)) < ENCODERS_SPEED_TIMEOUT)
                {
                    _encoderSpeed[index] = zlibs::utils::misc::constrain(static_cast<uint8_t>(_encoderSpeed[index] + ENCODER_SPEED_CHANGE[encAcceleration]),
                                                                         static_cast<uint8_t>(0),
                                                                         ENCODER_ACC_STEP_INC[encAcceleration]);
                }
                else
                {
                    _encoderSpeed[index] = 0;
                }
            }

            Descriptor descriptor;
            fillDescriptor(index, position, descriptor);

            sendMessage(index, position, descriptor);
        }
    }
}

void Encoders::sendMessage(size_t index, position_t position, Descriptor& descriptor)
{
    bool    send  = true;
    uint8_t steps = (_encoderSpeed[index] > 0) ? _encoderSpeed[index] : 1;

    switch (descriptor.type)
    {
    case type_t::CONTROL_CHANGE_7FH01H:
    {
        descriptor.signal.value = VAL_CONTROL_CHANGE_7FH01H[static_cast<uint8_t>(position)];
    }
    break;

    case type_t::CONTROL_CHANGE_3FH41H:
    {
        descriptor.signal.value = VAL_CONTROL_CHANGE_3FH41H[static_cast<uint8_t>(position)];
    }
    break;

    case type_t::CONTROL_CHANGE_41H01H:
    {
        descriptor.signal.value = VAL_CONTROL_CHANGE_41H01H[static_cast<uint8_t>(position)];
    }
    break;

    case type_t::PROGRAM_CHANGE:
    {
        if (position == position_t::CCW)
        {
            if (!MidiProgram.incrementProgram(descriptor.signal.channel, 1))
            {
                send = false;    // edge value reached, nothing more to send
            }
        }
        else
        {
            if (!MidiProgram.decrementProgram(descriptor.signal.channel, 1))
            {
                send = false;    // edge value reached, nothing more to send
            }
        }

        descriptor.signal.value = MidiProgram.program(descriptor.signal.channel);
    }
    break;

    case type_t::CONTROL_CHANGE:
    case type_t::PITCH_BEND:
    case type_t::NRPN_7BIT:
    case type_t::NRPN_14BIT:
    case type_t::CONTROL_CHANGE_14BIT:
    case type_t::SINGLE_NOTE_VARIABLE_VAL:
    {
        const bool USE_14BIT =
            ((descriptor.type == type_t::PITCH_BEND) || (descriptor.type == type_t::NRPN_14BIT) || (descriptor.type == type_t::CONTROL_CHANGE_14BIT));

        if (USE_14BIT && (steps > 1))
        {
            steps <<= 2;
        }

        if (position == position_t::CCW)
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
            case type_t::SINGLE_NOTE_VARIABLE_VAL:
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

        _value[index] = zlibs::utils::misc::constrain(static_cast<uint16_t>(_value[index]),
                                                      static_cast<uint16_t>(_database.read(database::Config::Section::encoder_t::LOWER_LIMIT, index)),
                                                      static_cast<uint16_t>(_database.read(database::Config::Section::encoder_t::UPPER_LIMIT, index)));

        if (descriptor.type == type_t::CONTROL_CHANGE_14BIT)
        {
            if (descriptor.signal.index >= 96)
            {
                // not allowed
                send = false;
            }
        }

        descriptor.signal.value = _value[index];
    }
    break;

    case type_t::SINGLE_NOTE_FIXED_VAL_BOTH_DIR:
    case type_t::SINGLE_NOTE_FIXED_VAL_ONE_DIR_0_OTHER_DIR:
    case type_t::TWO_NOTE_FIXED_VAL_BOTH_DIR:
    {
        descriptor.signal.value = _database.read(database::Config::Section::encoder_t::REPEATED_VALUE, index);

        if (descriptor.type == type_t::SINGLE_NOTE_FIXED_VAL_ONE_DIR_0_OTHER_DIR)
        {
            if (position == position_t::CCW)
            {
                descriptor.signal.value = 0;
            }
        }
    }
    break;

    case type_t::PRESET_CHANGE:
    {
        messaging::SystemSignal signal = {};
        signal.systemMessage           = (position == position_t::CW)
                                             ? messaging::systemMessage_t::PRESET_CHANGE_INC_REQ
                                             : messaging::systemMessage_t::PRESET_CHANGE_DEC_REQ;
        messaging::publish(signal);
        return;
    }
    break;

    case type_t::BPM_CHANGE:
    {
        if (position == position_t::CCW)
        {
            if (!Bpm.increment(1))
            {
                send = false;    // edge value reached, nothing more to send
            }
        }
        else
        {
            if (!Bpm.decrement(1))
            {
                send = false;    // edge value reached, nothing more to send
            }
        }
        return;
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
        messaging::publish(descriptor.signal);
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
position_t Encoders::read(size_t index, uint8_t pairState)
{
    auto retVal = position_t::STOPPED;
    pairState &= 0x03;

    // add new data

    bool process = true;

    // only process the data from encoder if there is a previous reading stored
    if (!zlibs::utils::misc::bit_read(_encoderData[index], 7))
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

void Encoders::fillDescriptor(size_t index, position_t position, Descriptor& descriptor)
{
    descriptor.type = static_cast<type_t>(_database.read(database::Config::Section::encoder_t::MODE, index));

    switch (descriptor.type)
    {
    case type_t::TWO_NOTE_FIXED_VAL_BOTH_DIR:
    {
        if (position == position_t::CCW)
        {
            descriptor.signal.index = _database.read(database::Config::Section::encoder_t::MIDI_ID_2, index);
        }
        else
        {
            descriptor.signal.index = _database.read(database::Config::Section::encoder_t::MIDI_ID_1, index);
        }
    }
    break;

    default:
    {
        descriptor.signal.index = _database.read(database::Config::Section::encoder_t::MIDI_ID_1, index);
    }
    break;
    }

    descriptor.signal.source         = messaging::MidiSource::Encoder;
    descriptor.signal.componentIndex = index;
    descriptor.signal.channel        = _database.read(database::Config::Section::encoder_t::CHANNEL, index);
    descriptor.signal.message        = INTERNAL_MSG_TO_MIDI_TYPE[static_cast<uint8_t>(descriptor.type)];
}

std::optional<uint8_t> Encoders::sysConfigGet(sys::Config::Section::encoder_t section, size_t index, uint16_t& value)
{
    uint32_t readValue = 0;

    auto result = _database.read(util::Conversion::SYS_2_DB_SECTION(section), index, readValue)
                      ? sys::Config::Status::ACK
                      : sys::Config::Status::ERROR_READ;

    if (result == sys::Config::Status::ACK)
    {
        if (section == sys::Config::Section::encoder_t::RESERVED_1)
        {
            return sys::Config::Status::ERROR_NOT_SUPPORTED;
        }
    }

    value = readValue;

    return result;
}

std::optional<uint8_t> Encoders::sysConfigSet(sys::Config::Section::encoder_t section, size_t index, uint16_t value)
{
    switch (section)
    {
    case sys::Config::Section::encoder_t::RESERVED_1:
        return sys::Config::Status::ERROR_NOT_SUPPORTED;

    default:
        break;
    }

    auto result = _database.update(util::Conversion::SYS_2_DB_SECTION(section), index, value)
                      ? sys::Config::Status::ACK
                      : sys::Config::Status::ERROR_WRITE;

    if (result == sys::Config::Status::ACK)
    {
        reset(index);
    }

    return result;
}

#endif
