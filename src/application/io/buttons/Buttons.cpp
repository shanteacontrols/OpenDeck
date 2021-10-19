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

#include "Buttons.h"
#include "io/common/Common.h"
#include "core/src/general/Helpers.h"
#include "core/src/general/Timing.h"

using namespace IO;

Buttons::Buttons(HWA&                     hwa,
                 Filter&                  filter,
                 Database&                database,
                 Util::MessageDispatcher& dispatcher)
    : _hwa(hwa)
    , _filter(filter)
    , _database(database)
    , _dispatcher(dispatcher)
{
    _dispatcher.listen(Util::MessageDispatcher::messageSource_t::analog,
                       Util::MessageDispatcher::listenType_t::forward,
                       [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                           size_t             index = dispatchMessage.componentIndex + MAX_NUMBER_OF_BUTTONS;
                           buttonDescriptor_t descriptor;
                           fillButtonDescriptor(index, descriptor);

                           // dispatchMessage.midiValue in this case contains state information only
                           processButton(index, dispatchMessage.midiValue, descriptor);
                       });

    _dispatcher.listen(Util::MessageDispatcher::messageSource_t::touchscreenButton,
                       Util::MessageDispatcher::listenType_t::forward,
                       [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                           size_t             index = dispatchMessage.componentIndex + MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG;
                           buttonDescriptor_t descriptor;
                           fillButtonDescriptor(index, descriptor);

                           // dispatchMessage.midiValue in this case contains state information only
                           processButton(index, dispatchMessage.midiValue, descriptor);
                       });
}

/// Continuously reads inputs from buttons and acts if necessary.
void Buttons::update(bool forceResend)
{
    for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
    {
        uint8_t  numberOfReadings = 0;
        uint32_t states           = 0;

        buttonDescriptor_t descriptor;
        fillButtonDescriptor(i, descriptor);

        if (!forceResend)
        {
            if (!_hwa.state(i, numberOfReadings, states))
                continue;

            // this filter will return amount of stable changed readings
            // and the states of those readings
            // latest reading is index 0
            if (!_filter.isFiltered(i, numberOfReadings, states))
                continue;

            for (uint8_t reading = 0; reading < numberOfReadings; reading++)
            {
                // when processing, newest sample has index 0
                // start from oldest reading which is in upper bits
                uint8_t processIndex = numberOfReadings - 1 - reading;
                bool    state        = (states >> processIndex) & 0x01;

                processButton(i, state, descriptor);
            }
        }
        else
        {
            if (descriptor.type == type_t::latching)
                sendMessage(i, latchingState(i), descriptor);
            else
                sendMessage(i, state(i), descriptor);
        }
    }
}

/// Handles changes in button states.
/// param [in]: index       Button index which has changed state.
/// param [in]: descriptor  Descriptor containing the entire configuration for the button.
void Buttons::processButton(size_t index, bool reading, buttonDescriptor_t& descriptor)
{
    // act on change of state only
    if (reading == state(index))
        return;

    setState(index, reading);

    // don't process messageType_t::none type of message
    if (descriptor.messageType != messageType_t::none)
    {
        if (descriptor.messageType == messageType_t::presetOpenDeck)
        {
            // change preset only on press
            if (reading)
            {
                // don't send off message once the preset is switched (in case this button has standard message type in switched preset)
                // pretend the button is already released
                setState(index, false);
                _database.setPreset(descriptor.dispatchMessage.midiIndex);
            }
        }
        else
        {
            bool sendMIDI = true;

            if (descriptor.type == type_t::latching)
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
                sendMessage(index, reading, descriptor);
        }
    }
}

/// Used to send MIDI message from specified button.
/// Used internally once the button state has been changed and processed.
/// param [in]: index           Button index which sends the message.
/// param [in]: descriptor      Structure holding all the information about button for specified index.
void Buttons::sendMessage(size_t index, bool state, buttonDescriptor_t& descriptor)
{
    bool send = true;

    if (state)
    {
        switch (descriptor.messageType)
        {
        case messageType_t::note:
        case messageType_t::controlChange:
        case messageType_t::controlChangeReset:
        case messageType_t::realTimeClock:
        case messageType_t::realTimeStart:
        case messageType_t::realTimeContinue:
        case messageType_t::realTimeStop:
        case messageType_t::realTimeActiveSensing:
        case messageType_t::realTimeSystemReset:
        case messageType_t::mmcPlay:
        case messageType_t::mmcStop:
        case messageType_t::mmcPause:
        case messageType_t::mmcRecord:
            break;

        case messageType_t::programChange:
        case messageType_t::programChangeInc:
        case messageType_t::programChangeDec:
        {
            descriptor.dispatchMessage.midiValue = 0;

            if (descriptor.messageType != messageType_t::programChange)
            {
                if (descriptor.messageType == messageType_t::programChangeInc)
                {
                    if (!Common::pcIncrement(descriptor.dispatchMessage.midiChannel))
                        send = false;
                }
                else
                {
                    if (!Common::pcDecrement(descriptor.dispatchMessage.midiChannel))
                        send = false;
                }

                descriptor.dispatchMessage.midiIndex = Common::program(descriptor.dispatchMessage.midiChannel);
            }
        }
        break;

        case messageType_t::multiValIncResetNote:
        {
            uint8_t currentValue = Common::currentValue(index);
            uint8_t value        = Common::valueInc(index, descriptor.dispatchMessage.midiValue, Common::incDecType_t::reset);

            if (currentValue != value)
            {
                if (!value)
                    descriptor.dispatchMessage.message = MIDI::messageType_t::noteOff;
                else
                    descriptor.dispatchMessage.message = MIDI::messageType_t::noteOn;

                descriptor.dispatchMessage.midiValue = currentValue;
            }
            else
            {
                send = false;
            }
        }
        break;

        case messageType_t::multiValIncDecNote:
        {
            uint8_t currentValue = Common::currentValue(index);
            uint8_t value        = Common::valueIncDec(index, descriptor.dispatchMessage.midiValue);

            if (currentValue != value)
            {
                if (!value)
                    descriptor.dispatchMessage.message = MIDI::messageType_t::noteOff;
                else
                    descriptor.dispatchMessage.message = MIDI::messageType_t::noteOn;

                descriptor.dispatchMessage.midiValue = currentValue;
            }
            else
            {
                send = false;
            }
        }
        break;

        case messageType_t::multiValIncResetCC:
        {
            uint8_t currentValue = Common::currentValue(index);
            uint8_t value        = Common::valueInc(index, descriptor.dispatchMessage.midiValue, Common::incDecType_t::reset);

            if (currentValue != value)
                descriptor.dispatchMessage.midiValue = currentValue;
            else
                send = false;
        }
        break;

        case messageType_t::multiValIncDecCC:
        {
            uint8_t currentValue = Common::currentValue(index);
            uint8_t value        = Common::valueIncDec(index, descriptor.dispatchMessage.midiValue);

            if (currentValue != value)
                descriptor.dispatchMessage.midiValue = currentValue;
            else
                send = false;
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
        case messageType_t::note:
        {
            descriptor.dispatchMessage.midiValue = 0;
            descriptor.dispatchMessage.message   = MIDI::messageType_t::noteOff;
        }
        break;

        case messageType_t::controlChangeReset:
        {
            descriptor.dispatchMessage.midiValue = 0;
        }
        break;

        case messageType_t::mmcRecord:
        {
            descriptor.dispatchMessage.message = MIDI::messageType_t::mmcRecordStop;
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
        _dispatcher.notify(Util::MessageDispatcher::messageSource_t::buttons,
                           descriptor.dispatchMessage,
                           Util::MessageDispatcher::listenType_t::nonFwd);
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
    descriptor.type                           = static_cast<type_t>(_database.read(Database::Section::button_t::type, index));
    descriptor.messageType                    = static_cast<messageType_t>(_database.read(Database::Section::button_t::midiMessage, index));
    descriptor.dispatchMessage.componentIndex = index;
    descriptor.dispatchMessage.midiChannel    = _database.read(Database::Section::button_t::midiChannel, index);
    descriptor.dispatchMessage.midiIndex      = _database.read(Database::Section::button_t::midiID, index);
    descriptor.dispatchMessage.midiValue      = _database.read(Database::Section::button_t::velocity, index);

    // overwrite type under certain conditions
    switch (descriptor.messageType)
    {
    case messageType_t::programChange:
    case messageType_t::programChangeInc:
    case messageType_t::programChangeDec:
    case messageType_t::mmcPlay:
    case messageType_t::mmcStop:
    case messageType_t::mmcPause:
    case messageType_t::controlChange:
    case messageType_t::realTimeClock:
    case messageType_t::realTimeStart:
    case messageType_t::realTimeContinue:
    case messageType_t::realTimeStop:
    case messageType_t::realTimeActiveSensing:
    case messageType_t::realTimeSystemReset:
    case messageType_t::multiValIncResetNote:
    case messageType_t::multiValIncDecNote:
    case messageType_t::multiValIncResetCC:
    case messageType_t::multiValIncDecCC:
    {
        descriptor.type = type_t::momentary;
    }
    break;

    case messageType_t::mmcRecord:
    {
        descriptor.type = type_t::latching;
    }
    break;

    case messageType_t::presetOpenDeck:
    {
        descriptor.type = type_t::momentary;
    }
    break;

    default:
        break;
    }

    descriptor.dispatchMessage.message = _internalMsgToMIDIType[static_cast<uint8_t>(descriptor.messageType)];
}