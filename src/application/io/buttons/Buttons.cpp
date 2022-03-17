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

#include "Buttons.h"
#include "sysex/src/SysExConf.h"
#include "system/Config.h"

#ifdef BUTTONS_SUPPORTED

#include "core/src/general/Helpers.h"
#include "core/src/general/Timing.h"
#include "util/conversion/Conversion.h"
#include "util/configurable/Configurable.h"

using namespace IO;

Buttons::Buttons(HWA&                hwa,
                 Filter&             filter,
                 Database::Instance& database)
    : _hwa(hwa)
    , _filter(filter)
    , _database(database)
{
    MIDIDispatcher.listen(Messaging::eventType_t::ANALOG_BUTTON,
                          [this](const Messaging::event_t& event)
                          {
                              size_t             index = event.componentIndex + Collection::startIndex(GROUP_ANALOG_INPUTS);
                              buttonDescriptor_t descriptor;
                              fillButtonDescriptor(index, descriptor);

                              // event.value in this case contains state information only
                              processButton(index, event.value, descriptor);
                          });

    MIDIDispatcher.listen(Messaging::eventType_t::TOUCHSCREEN_BUTTON,
                          [this](const Messaging::event_t& event)
                          {
                              size_t index = event.componentIndex + Collection::startIndex(GROUP_TOUCHSCREEN_COMPONENTS);

                              buttonDescriptor_t descriptor;
                              fillButtonDescriptor(index, descriptor);

                              // event.value in this case contains state information only
                              processButton(index, event.value, descriptor);
                          });

    MIDIDispatcher.listen(Messaging::eventType_t::SYSTEM,
                          [this](const Messaging::event_t& event)
                          {
                              switch (event.componentIndex)
                              {
                              case static_cast<uint8_t>(Messaging::systemMessage_t::FORCE_IO_REFRESH):
                              {
                                  updateAll(true);
                              }
                              break;

                              default:
                                  break;
                              }
                          });

    ConfigHandler.registerConfig(
        System::Config::block_t::BUTTONS,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sysConfigGet(static_cast<System::Config::Section::button_t>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sysConfigSet(static_cast<System::Config::Section::button_t>(section), index, value);
        });
}

bool Buttons::init()
{
    for (size_t i = 0; i < Collection::size(); i++)
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

    buttonDescriptor_t descriptor;

    uint8_t  numberOfReadings = 0;
    uint32_t states           = 0;

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

        fillButtonDescriptor(index, descriptor);

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
        fillButtonDescriptor(index, descriptor);

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
    for (size_t i = 0; i < Collection::size(GROUP_DIGITAL_INPUTS); i++)
    {
        updateSingle(i, forceRefresh);
    }
}

size_t Buttons::maxComponentUpdateIndex()
{
    return Collection::size(GROUP_DIGITAL_INPUTS);
}

/// Handles changes in button states.
/// param [in]: index       Button index which has changed state.
/// param [in]: descriptor  Descriptor containing the entire configuration for the button.
void Buttons::processButton(size_t index, bool reading, buttonDescriptor_t& descriptor)
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
        if (descriptor.messageType == messageType_t::PRESET_OPEN_DECK)
        {
            // change preset only on press
            if (reading)
            {
                // don't send off message once the preset is switched (in case this button has standard message type in switched preset)
                // pretend the button is already released
                setState(index, false);
                _database.setPreset(descriptor.event.index);
            }
        }
        else
        {
            bool sendMIDI = true;

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
                    sendMIDI = false;
                }
            }

            if (sendMIDI)
            {
                sendMessage(index, reading, descriptor);
            }
        }
    }
}

/// Used to send MIDI message from specified button.
/// Used internally once the button state has been changed and processed.
/// param [in]: index           Button index which sends the message.
/// param [in]: descriptor      Structure holding all the information about button for specified index.
void Buttons::sendMessage(size_t index, bool state, buttonDescriptor_t& descriptor)
{
    bool send      = true;
    auto eventType = Messaging::eventType_t::BUTTON;

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
            break;

        case messageType_t::DMX:
        {
            descriptor.event.index = 0;    // irrelevant
            eventType              = Messaging::eventType_t::DMX_BUTTON;
        }
        break;

        case messageType_t::PROGRAM_CHANGE:
        case messageType_t::PROGRAM_CHANGE_INC:
        case messageType_t::PROGRAM_CHANGE_DEC:
        {
            descriptor.event.value = 0;

            if (descriptor.messageType != messageType_t::PROGRAM_CHANGE)
            {
                if (descriptor.messageType == messageType_t::PROGRAM_CHANGE_INC)
                {
                    if (!Common::pcIncrement(descriptor.event.channel))
                    {
                        send = false;
                    }
                }
                else
                {
                    if (!Common::pcDecrement(descriptor.event.channel))
                    {
                        send = false;
                    }
                }

                descriptor.event.index = Common::program(descriptor.event.channel);
            }
        }
        break;

        case messageType_t::MULTI_VAL_INC_RESET_NOTE:
        {
            uint8_t currentValue = Common::currentValue(index);
            uint8_t value        = Common::valueInc(index, descriptor.event.value, Common::incDecType_t::RESET);

            if (currentValue != value)
            {
                if (!value)
                {
                    descriptor.event.message = MIDI::messageType_t::NOTE_OFF;
                }
                else
                {
                    descriptor.event.message = MIDI::messageType_t::NOTE_ON;
                }

                descriptor.event.value = currentValue;
            }
            else
            {
                send = false;
            }
        }
        break;

        case messageType_t::MULTI_VAL_INC_DEC_NOTE:
        {
            uint8_t currentValue = Common::currentValue(index);
            uint8_t value        = Common::valueIncDec(index, descriptor.event.value);

            if (currentValue != value)
            {
                if (!value)
                {
                    descriptor.event.message = MIDI::messageType_t::NOTE_OFF;
                }
                else
                {
                    descriptor.event.message = MIDI::messageType_t::NOTE_ON;
                }

                descriptor.event.value = currentValue;
            }
            else
            {
                send = false;
            }
        }
        break;

        case messageType_t::MULTI_VAL_INC_RESET_CC:
        {
            uint8_t currentValue = Common::currentValue(index);
            uint8_t value        = Common::valueInc(index, descriptor.event.value, Common::incDecType_t::RESET);

            if (currentValue != value)
            {
                descriptor.event.value = currentValue;
            }
            else
            {
                send = false;
            }
        }
        break;

        case messageType_t::MULTI_VAL_INC_DEC_CC:
        {
            uint8_t currentValue = Common::currentValue(index);
            uint8_t value        = Common::valueIncDec(index, descriptor.event.value);

            if (currentValue != value)
            {
                descriptor.event.value = currentValue;
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
            descriptor.event.message = MIDI::messageType_t::NOTE_OFF;
        }
        break;

        case messageType_t::CONTROL_CHANGE0_ONLY:
        {
            descriptor.event.value = 0;
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
        case messageType_t::NOTE_OFF_ONLY:
        {
            descriptor.event.value   = 0;
            descriptor.event.message = MIDI::messageType_t::NOTE_OFF;
        }
        break;

        case messageType_t::CONTROL_CHANGE_RESET:
        case messageType_t::CONTROL_CHANGE0_ONLY:
        {
            descriptor.event.value = 0;
        }
        break;

        case messageType_t::MMC_RECORD:
        {
            descriptor.event.message = MIDI::messageType_t::MMC_RECORD_STOP;
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
        MIDIDispatcher.notify(eventType, descriptor.event);
    }
}

/// Updates current state of button.
/// param [in]: index       Button for which state is being changed.
/// param [in]: state       New button state (true/pressed, false/released).
void Buttons::setState(size_t index, bool state)
{
    uint8_t arrayIndex  = index / 8;
    uint8_t buttonIndex = index - 8 * arrayIndex;

    BIT_WRITE(_buttonPressed[arrayIndex], buttonIndex, state);
}

/// Checks for last button state.
/// param [in]: index    Button index for which previous state is being checked.
/// returns: True if last state was on/pressed, false otherwise.
bool Buttons::state(size_t index)
{
    uint8_t arrayIndex  = index / 8;
    uint8_t buttonIndex = index - 8 * arrayIndex;

    return BIT_READ(_buttonPressed[arrayIndex], buttonIndex);
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
    uint8_t arrayIndex  = index / 8;
    uint8_t buttonIndex = index - 8 * arrayIndex;

    BIT_WRITE(_lastLatchingState[arrayIndex], buttonIndex, state);
}

/// Checks for last latching button state.
/// param [in]: index    Button index for which previous latching state is being checked.
/// returns: True if last state was on/pressed, false otherwise.
bool Buttons::latchingState(size_t index)
{
    uint8_t arrayIndex  = index / 8;
    uint8_t buttonIndex = index - 8 * arrayIndex;

    return BIT_READ(_lastLatchingState[arrayIndex], buttonIndex);
}

/// Resets the current state of the specified button.
/// param [in]: index    Button for which to reset state.
void Buttons::reset(size_t index)
{
    setState(index, false);
    setLatchingState(index, false);
}

void Buttons::fillButtonDescriptor(size_t index, buttonDescriptor_t& descriptor)
{
    descriptor.type                 = static_cast<type_t>(_database.read(Database::Config::Section::button_t::TYPE, index));
    descriptor.messageType          = static_cast<messageType_t>(_database.read(Database::Config::Section::button_t::MESSAGE_TYPE, index));
    descriptor.event.componentIndex = index;
    descriptor.event.channel        = _database.read(Database::Config::Section::button_t::CHANNEL, index);
    descriptor.event.index          = _database.read(Database::Config::Section::button_t::MIDI_ID, index);
    descriptor.event.value          = _database.read(Database::Config::Section::button_t::VALUE, index);

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
    case messageType_t::DMX:
    {
        descriptor.type = type_t::MOMENTARY;
    }
    break;

    case messageType_t::MMC_RECORD:
    case messageType_t::NOTE_OFF_ONLY:
    case messageType_t::CONTROL_CHANGE0_ONLY:
    {
        descriptor.type = type_t::LATCHING;
    }
    break;

    case messageType_t::PRESET_OPEN_DECK:
    {
        descriptor.type = type_t::MOMENTARY;
    }
    break;

    default:
        break;
    }

    descriptor.event.message = INTERNAL_MSG_TO_MIDI_TYPE[static_cast<uint8_t>(descriptor.messageType)];
}

bool Buttons::state(size_t index, uint8_t& numberOfReadings, uint32_t& states)
{
    // if encoder under this index is enabled, just return false state each time
    if (_database.read(Database::Config::Section::encoder_t::ENABLE, _hwa.buttonToEncoderIndex(index)))
    {
        return false;
    }

    return _hwa.state(index, numberOfReadings, states);
}

std::optional<uint8_t> Buttons::sysConfigGet(System::Config::Section::button_t section, size_t index, uint16_t& value)
{
    int32_t readValue;
    auto    result = _database.read(Util::Conversion::sys2DBsection(section), index, readValue) ? System::Config::status_t::ACK : System::Config::status_t::ERROR_READ;
    value          = readValue;

    return result;
}

std::optional<uint8_t> Buttons::sysConfigSet(System::Config::Section::button_t section, size_t index, uint16_t value)
{
    auto result = _database.update(Util::Conversion::sys2DBsection(section), index, value) ? System::Config::status_t::ACK : System::Config::status_t::ERROR_WRITE;

    if (result == System::Config::status_t::ACK)
    {
        if (
            (section == System::Config::Section::button_t::TYPE) ||
            (section == System::Config::Section::button_t::MESSAGE_TYPE))
        {
            reset(index);
        }
    }

    return result;
}

#endif