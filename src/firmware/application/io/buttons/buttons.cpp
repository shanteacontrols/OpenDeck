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

#include "buttons.h"
#include "application/system/config.h"
#include "application/global/midi_program.h"
#include "application/global/bpm.h"
#include "application/util/conversion/conversion.h"
#include "application/util/configurable/configurable.h"

#ifdef BUTTONS_SUPPORTED

#include "core/mcu.h"
#include "core/util/util.h"

using namespace io::buttons;

Buttons::Buttons(Hwa&      hwa,
                 Filter&   filter,
                 Database& database)
    : _hwa(hwa)
    , _filter(filter)
    , _database(database)
{
    MidiDispatcher.listen(messaging::eventType_t::ANALOG_BUTTON,
                          [this](const messaging::Event& event)
                          {
                              size_t     index = event.componentIndex + Collection::START_INDEX(GROUP_ANALOG_INPUTS);
                              Descriptor descriptor;
                              fillDescriptor(index, descriptor);

                              if (!event.forcedRefresh)
                              {
                                  // event.value in this case contains state information only
                                  processButton(index, event.value, descriptor);
                              }
                              else
                              {
                                  if (descriptor.type == type_t::LATCHING)
                                  {
                                      sendMessage(index, latchingState(index), descriptor);
                                  }
                                  else
                                  {
                                      sendMessage(index, state(index), descriptor);
                                  }
                              }
                          });

    MidiDispatcher.listen(messaging::eventType_t::TOUCHSCREEN_BUTTON,
                          [this](const messaging::Event& event)
                          {
                              size_t index = event.componentIndex + Collection::START_INDEX(GROUP_TOUCHSCREEN_COMPONENTS);

                              Descriptor descriptor;
                              fillDescriptor(index, descriptor);

                              // event.value in this case contains state information only
                              processButton(index, event.value, descriptor);
                          });

    MidiDispatcher.listen(messaging::eventType_t::SYSTEM,
                          [this](const messaging::Event& event)
                          {
                              switch (event.systemMessage)
                              {
                              case messaging::systemMessage_t::FORCE_IO_REFRESH:
                              {
                                  updateAll(true);
                              }
                              break;

                              default:
                                  break;
                              }
                          });

    ConfigHandler.registerConfig(
        sys::Config::block_t::BUTTONS,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sysConfigGet(static_cast<sys::Config::Section::button_t>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sysConfigSet(static_cast<sys::Config::Section::button_t>(section), index, value);
        });
}

bool Buttons::init()
{
    for (size_t i = 0; i < Collection::SIZE(); i++)
    {
        reset(i);
    }

    return true;
}

void Buttons::updateSingle(size_t index, bool forceRefresh)
{
    if (index >= maxComponentUpdateIndex())
    {
        return;
    }

    Descriptor descriptor;

    uint8_t  numberOfReadings = 0;
    uint16_t states           = 0;

    if (!forceRefresh)
    {
        if (!state(index, numberOfReadings, states))
        {
            return;
        }

        // this filter will return amount of stable changed readings
        // and the states of those readings
        // latest reading is index 0
        if (!_filter.isFiltered(index, numberOfReadings, states))
        {
            return;
        }

        fillDescriptor(index, descriptor);

        for (uint8_t reading = 0; reading < numberOfReadings; reading++)
        {
            // when processing, newest sample has index 0
            // start from oldest reading which is in upper bits
            uint8_t processIndex = numberOfReadings - 1 - reading;
            bool    state        = (states >> processIndex) & 0x01;

            processButton(index, state, descriptor);
        }
    }
    else
    {
        fillDescriptor(index, descriptor);

        if (descriptor.type == type_t::LATCHING)
        {
            sendMessage(index, latchingState(index), descriptor);
        }
        else
        {
            sendMessage(index, state(index), descriptor);
        }
    }
}

void Buttons::updateAll(bool forceRefresh)
{
    for (size_t i = 0; i < Collection::SIZE(GROUP_DIGITAL_INPUTS); i++)
    {
        updateSingle(i, forceRefresh);
    }
}

size_t Buttons::maxComponentUpdateIndex()
{
    return Collection::SIZE(GROUP_DIGITAL_INPUTS);
}

/// Handles changes in button states.
/// param [in]: index       Button index which has changed state.
/// param [in]: descriptor  Descriptor containing the entire configuration for the button.
void Buttons::processButton(size_t index, bool reading, Descriptor& descriptor)
{
    // act on change of state only
    if (reading == state(index))
    {
        return;
    }

    setState(index, reading);

    // don't process messageType_t::NONE type of message
    if (descriptor.messageType != messageType_t::NONE)
    {
        bool send = true;

        if (descriptor.type == type_t::LATCHING)
        {
            // act on press only
            if (reading)
            {
                if (latchingState(index))
                {
                    setLatchingState(index, false);
                    // overwrite before processing
                    reading = false;
                }
                else
                {
                    setLatchingState(index, true);
                    reading = true;
                }
            }
            else
            {
                send = false;
            }
        }

        if (send)
        {
            sendMessage(index, reading, descriptor);
        }
    }
}

/// Used to send MIDI message from specified button.
/// Used internally once the button state has been changed and processed.
/// param [in]: index           Button index which sends the message.
/// param [in]: descriptor      Structure holding all the information about button for specified index.
void Buttons::sendMessage(size_t index, bool state, Descriptor& descriptor)
{
    bool send      = true;
    auto eventType = messaging::eventType_t::BUTTON;

    if (state)
    {
        switch (descriptor.messageType)
        {
        case messageType_t::NOTE:
        case messageType_t::CONTROL_CHANGE:
        case messageType_t::CONTROL_CHANGE_RESET:
        case messageType_t::REAL_TIME_CLOCK:
        case messageType_t::REAL_TIME_START:
        case messageType_t::REAL_TIME_CONTINUE:
        case messageType_t::REAL_TIME_STOP:
        case messageType_t::REAL_TIME_ACTIVE_SENSING:
        case messageType_t::REAL_TIME_SYSTEM_RESET:
        case messageType_t::MMC_PLAY:
        case messageType_t::MMC_STOP:
        case messageType_t::MMC_PAUSE:
        case messageType_t::MMC_RECORD:
        case messageType_t::MMC_PLAY_STOP:
            break;

        case messageType_t::PROGRAM_CHANGE:
        {
            descriptor.event.value = 0;
            descriptor.event.index += MidiProgram.offset();
            descriptor.event.index &= 0x7F;
        }
        break;

        case messageType_t::PROGRAM_CHANGE_INC:
        {
            descriptor.event.value = 0;

            if (!MidiProgram.incrementProgram(descriptor.event.channel, 1))
            {
                send = false;
            }

            descriptor.event.index = MidiProgram.program(descriptor.event.channel);
        }
        break;

        case messageType_t::PROGRAM_CHANGE_DEC:
        {
            descriptor.event.value = 0;

            if (!MidiProgram.decrementProgram(descriptor.event.channel, 1))
            {
                send = false;
            }

            descriptor.event.index = MidiProgram.program(descriptor.event.channel);
        }
        break;

        case messageType_t::MULTI_VAL_INC_RESET_NOTE:
        {
            auto newValue = ValueIncDecMIDI7Bit::increment(_incDecValue[index],
                                                           descriptor.event.value,
                                                           ValueIncDecMIDI7Bit::type_t::OVERFLOW);

            if (newValue != _incDecValue[index])
            {
                if (!newValue)
                {
                    descriptor.event.message = midi::messageType_t::NOTE_OFF;
                }
                else
                {
                    descriptor.event.message = midi::messageType_t::NOTE_ON;
                }

                _incDecValue[index]    = newValue;
                descriptor.event.value = newValue;
            }
            else
            {
                send = false;
            }
        }
        break;

        case messageType_t::MULTI_VAL_INC_DEC_NOTE:
        {
            auto newValue = ValueIncDecMIDI7Bit::increment(_incDecValue[index],
                                                           descriptor.event.value,
                                                           ValueIncDecMIDI7Bit::type_t::EDGE);

            if (newValue != _incDecValue[index])
            {
                if (!newValue)
                {
                    descriptor.event.message = midi::messageType_t::NOTE_OFF;
                }
                else
                {
                    descriptor.event.message = midi::messageType_t::NOTE_ON;
                }

                _incDecValue[index]    = newValue;
                descriptor.event.value = newValue;
            }
            else
            {
                send = false;
            }
        }
        break;

        case messageType_t::MULTI_VAL_INC_RESET_CC:
        {
            auto newValue = ValueIncDecMIDI7Bit::increment(_incDecValue[index],
                                                           descriptor.event.value,
                                                           ValueIncDecMIDI7Bit::type_t::OVERFLOW);

            if (newValue != _incDecValue[index])
            {
                _incDecValue[index]    = newValue;
                descriptor.event.value = newValue;
            }
            else
            {
                send = false;
            }
        }
        break;

        case messageType_t::MULTI_VAL_INC_DEC_CC:
        {
            auto newValue = ValueIncDecMIDI7Bit::increment(_incDecValue[index],
                                                           descriptor.event.value,
                                                           ValueIncDecMIDI7Bit::type_t::EDGE);

            if (newValue != _incDecValue[index])
            {
                _incDecValue[index]    = newValue;
                descriptor.event.value = newValue;
            }
            else
            {
                send = false;
            }
        }
        break;

        case messageType_t::NOTE_OFF_ONLY:
        {
            descriptor.event.value   = 0;
            descriptor.event.message = midi::messageType_t::NOTE_OFF;
        }
        break;

        case messageType_t::CONTROL_CHANGE0_ONLY:
        {
            descriptor.event.value = 0;
        }
        break;

        case messageType_t::PROGRAM_CHANGE_OFFSET_INC:
        {
            MidiProgram.incrementOffset(descriptor.event.value);
        }
        break;

        case messageType_t::PROGRAM_CHANGE_OFFSET_DEC:
        {
            MidiProgram.decrementOffset(descriptor.event.value);
        }
        break;

        case messageType_t::PRESET_CHANGE:
        {
            eventType                      = messaging::eventType_t::SYSTEM;
            descriptor.event.systemMessage = messaging::systemMessage_t::PRESET_CHANGE_DIRECT_REQ;
        }
        break;

        case messageType_t::BPM_INC:
        {
            descriptor.event.value = 0;

            if (!Bpm.increment(1))
            {
                send = false;
            }

            descriptor.event.index = Bpm.value();
        }
        break;

        case messageType_t::BPM_DEC:
        {
            descriptor.event.value = 0;

            if (!Bpm.decrement(1))
            {
                send = false;
            }

            descriptor.event.index = Bpm.value();
        }
        break;

        default:
        {
            send = false;
        }
        break;
        }
    }
    else
    {
        switch (descriptor.messageType)
        {
        case messageType_t::NOTE:
        {
            descriptor.event.value   = 0;
            descriptor.event.message = midi::messageType_t::NOTE_OFF;
        }
        break;

        case messageType_t::CONTROL_CHANGE_RESET:
        {
            descriptor.event.value = 0;
        }
        break;

        case messageType_t::MMC_RECORD:
        {
            descriptor.event.message = midi::messageType_t::MMC_RECORD_STOP;
        }
        break;

        case messageType_t::MMC_PLAY_STOP:
        {
            descriptor.event.message = midi::messageType_t::MMC_STOP;
        }
        break;

        default:
        {
            send = false;
        }
        break;
        }
    }

    if (send)
    {
        MidiDispatcher.notify(eventType, descriptor.event);
    }
}

/// Updates current state of button.
/// param [in]: index       Button for which state is being changed.
/// param [in]: state       New button state (true/pressed, false/released).
void Buttons::setState(size_t index, bool state)
{
    uint8_t arrayIndex = index / 8;
    uint8_t bit        = index - 8 * arrayIndex;

    core::util::BIT_WRITE(_buttonPressed[arrayIndex], bit, state);
}

/// Checks for last button state.
/// param [in]: index    Button index for which previous state is being checked.
/// returns: True if last state was on/pressed, false otherwise.
bool Buttons::state(size_t index)
{
    uint8_t arrayIndex = index / 8;
    uint8_t bit        = index - 8 * arrayIndex;

    return core::util::BIT_READ(_buttonPressed[arrayIndex], bit);
}

/// Updates current state of latching button.
/// Used only for latching buttons where new state which should be sent differs
/// from last one, for instance when sending MIDI note on on first press (latching
/// state: true), and note off on second (latching state: false).
/// State should be stored in variable because unlike momentary buttons, state of
/// latching buttons doesn't necessarrily match current "real" state of button since events
/// for latching buttons are sent only on presses.
/// param [in]: index    Button for which state is being changed.
/// param [in]: state       New latching state.
void Buttons::setLatchingState(size_t index, bool state)
{
    uint8_t arrayIndex = index / 8;
    uint8_t bit        = index - 8 * arrayIndex;

    core::util::BIT_WRITE(_lastLatchingState[arrayIndex], bit, state);
}

/// Checks for last latching button state.
/// param [in]: index    Button index for which previous latching state is being checked.
/// returns: True if last state was on/pressed, false otherwise.
bool Buttons::latchingState(size_t index)
{
    uint8_t arrayIndex = index / 8;
    uint8_t bit        = index - 8 * arrayIndex;

    return core::util::BIT_READ(_lastLatchingState[arrayIndex], bit);
}

/// Resets the current state of the specified button.
/// param [in]: index    Button for which to reset state.
void Buttons::reset(size_t index)
{
    setState(index, false);
    setLatchingState(index, false);
}

void Buttons::fillDescriptor(size_t index, Descriptor& descriptor)
{
    descriptor.type                 = static_cast<type_t>(_database.read(database::Config::Section::button_t::TYPE, index));
    descriptor.messageType          = static_cast<messageType_t>(_database.read(database::Config::Section::button_t::MESSAGE_TYPE, index));
    descriptor.event.componentIndex = index;
    descriptor.event.channel        = _database.read(database::Config::Section::button_t::CHANNEL, index);
    descriptor.event.index          = _database.read(database::Config::Section::button_t::MIDI_ID, index);
    descriptor.event.value          = _database.read(database::Config::Section::button_t::VALUE, index);

    // overwrite type under certain conditions
    switch (descriptor.messageType)
    {
    case messageType_t::PROGRAM_CHANGE:
    case messageType_t::PROGRAM_CHANGE_INC:
    case messageType_t::PROGRAM_CHANGE_DEC:
    case messageType_t::MMC_PLAY:
    case messageType_t::MMC_STOP:
    case messageType_t::MMC_PAUSE:
    case messageType_t::CONTROL_CHANGE:
    case messageType_t::REAL_TIME_CLOCK:
    case messageType_t::REAL_TIME_START:
    case messageType_t::REAL_TIME_CONTINUE:
    case messageType_t::REAL_TIME_STOP:
    case messageType_t::REAL_TIME_ACTIVE_SENSING:
    case messageType_t::REAL_TIME_SYSTEM_RESET:
    case messageType_t::MULTI_VAL_INC_RESET_NOTE:
    case messageType_t::MULTI_VAL_INC_DEC_NOTE:
    case messageType_t::MULTI_VAL_INC_RESET_CC:
    case messageType_t::MULTI_VAL_INC_DEC_CC:
    case messageType_t::PRESET_CHANGE:
    case messageType_t::PROGRAM_CHANGE_OFFSET_INC:
    case messageType_t::PROGRAM_CHANGE_OFFSET_DEC:
    case messageType_t::NOTE_OFF_ONLY:
    case messageType_t::CONTROL_CHANGE0_ONLY:
    case messageType_t::BPM_INC:
    case messageType_t::BPM_DEC:
    {
        descriptor.type = type_t::MOMENTARY;
    }
    break;

    case messageType_t::MMC_RECORD:
    case messageType_t::MMC_PLAY_STOP:
    {
        descriptor.type = type_t::LATCHING;
    }
    break;

    default:
        break;
    }

    descriptor.event.message = INTERNAL_MSG_TO_MIDI_TYPE[static_cast<uint8_t>(descriptor.messageType)];
}

bool Buttons::state(size_t index, uint8_t& numberOfReadings, uint16_t& states)
{
    // if encoder under this index is enabled, just return false state each time
    if (_database.read(database::Config::Section::encoder_t::ENABLE, _hwa.buttonToEncoderIndex(index)))
    {
        return false;
    }

    return _hwa.state(index, numberOfReadings, states);
}

std::optional<uint8_t> Buttons::sysConfigGet(sys::Config::Section::button_t section, size_t index, uint16_t& value)
{
    uint32_t readValue;

    auto result = _database.read(util::Conversion::SYS_2_DB_SECTION(section), index, readValue)
                      ? sys::Config::Status::ACK
                      : sys::Config::Status::ERROR_READ;

    value = readValue;

    return result;
}

std::optional<uint8_t> Buttons::sysConfigSet(sys::Config::Section::button_t section, size_t index, uint16_t value)
{
    auto result = _database.update(util::Conversion::SYS_2_DB_SECTION(section), index, value)
                      ? sys::Config::Status::ACK
                      : sys::Config::Status::ERROR_WRITE;

    if (result == sys::Config::Status::ACK)
    {
        if (
            (section == sys::Config::Section::button_t::TYPE) ||
            (section == sys::Config::Section::button_t::MESSAGE_TYPE))
        {
            reset(index);
        }
    }

    return result;
}

#endif