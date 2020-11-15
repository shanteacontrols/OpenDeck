/*

Copyright 2015-2020 Igor Petrovic

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

using namespace IO;

///
/// \brief Continuously reads inputs from buttons and acts if necessary.
///
void Buttons::update()
{
    for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
    {
        bool state;

        if (!filter.isFiltered(i, hwa.state(i), state))
            continue;

        processButton(i, state);
    }
}

///
/// \brief Handles changes in button states.
/// @param [in] buttonID    Button index which has changed state.
/// @param [in] state       Current button state.
///
void Buttons::processButton(uint8_t buttonID, bool state)
{
    buttonMessageDescriptor_t descriptor;

    descriptor.messageType = static_cast<messageType_t>(database.read(Database::Section::button_t::midiMessage, buttonID));
    descriptor.note        = database.read(Database::Section::button_t::midiID, buttonID);
    descriptor.channel     = database.read(Database::Section::button_t::midiChannel, buttonID);
    descriptor.velocity    = database.read(Database::Section::button_t::velocity, buttonID);

    auto type = static_cast<type_t>(database.read(Database::Section::button_t::type, buttonID));

    //act on change of state only
    if (state == getButtonState(buttonID))
        return;

    setButtonState(buttonID, state);

    //don't process messageType_t::none type of message
    if (descriptor.messageType != messageType_t::none)
    {
        bool sendMIDI = true;

        //overwrite type under certain conditions
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
            type = type_t::momentary;
            break;

        case messageType_t::mmcRecord:
            type = type_t::latching;
            break;

        case messageType_t::presetOpenDeck:
            type     = type_t::momentary;
            sendMIDI = false;
            break;

        default:
            break;
        }

        if (type == type_t::latching)
        {
            //act on press only
            if (state)
            {
                if (getLatchingState(buttonID))
                {
                    setLatchingState(buttonID, false);
                    //overwrite before processing
                    state = false;
                }
                else
                {
                    setLatchingState(buttonID, true);
                    state = true;
                }
            }
            else
            {
                sendMIDI = false;
            }
        }

        if (sendMIDI)
        {
            sendMessage(buttonID, state, descriptor);
        }
        else if (descriptor.messageType == messageType_t::presetOpenDeck)
        {
            //change preset only on press
            if (state)
            {
                uint8_t preset = database.read(Database::Section::button_t::midiID, buttonID);
                database.setPreset(preset);
            }
        }
    }

    cInfo.send(Database::block_t::buttons, buttonID);
}

///
/// \brief Used to send MIDI message from specified button.
/// Used internally once the button state has been changed and processed.
/// @param [in] buttonID        Button ID which sends the message.
/// @param [in] state           Button state (true/pressed, false/released).
/// @param [in] buttonMessage   Type of MIDI message to send. If unspecified, message type is read from database.
///
void Buttons::sendMessage(uint8_t buttonID, bool state, buttonMessageDescriptor_t& descriptor)
{
    if (descriptor.messageType == messageType_t::AMOUNT)
        descriptor.messageType = static_cast<messageType_t>(database.read(Database::Section::button_t::midiMessage, buttonID));

    mmcArray[2] = descriptor.note;    //use midi note as channel id for transport control

    bool send = true;

    if (state)
    {
        switch (descriptor.messageType)
        {
        case messageType_t::note:
        {
            midi.sendNoteOn(descriptor.note, descriptor.velocity, descriptor.channel);
#ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::noteOn, descriptor.note, descriptor.velocity, descriptor.channel + 1);
#endif
            leds.midiToState(MIDI::messageType_t::noteOn, descriptor.note, descriptor.velocity, descriptor.channel, true);
        }
        break;

        case messageType_t::programChange:
        case messageType_t::programChangeInc:
        case messageType_t::programChangeDec:
        {
            if (descriptor.messageType != messageType_t::programChange)
            {
                if (descriptor.messageType == messageType_t::programChangeInc)
                {
                    if (!Common::pcIncrement(descriptor.channel))
                        send = false;
                }
                else
                {
                    if (!Common::pcDecrement(descriptor.channel))
                        send = false;
                }

                descriptor.note = Common::program(descriptor.channel);
            }

            if (send)
            {
                midi.sendProgramChange(descriptor.note, descriptor.channel);
                leds.midiToState(MIDI::messageType_t::programChange, descriptor.note, 0, descriptor.channel, true);
#ifdef DISPLAY_SUPPORTED
                display.displayMIDIevent(Display::eventType_t::out, Display::event_t::programChange, descriptor.note, 0, descriptor.channel + 1);
#endif
            }
        }
        break;

        case messageType_t::controlChange:
        case messageType_t::controlChangeReset:
        {
            midi.sendControlChange(descriptor.note, descriptor.velocity, descriptor.channel);
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::controlChange, descriptor.note, descriptor.velocity, descriptor.channel + 1);
            leds.midiToState(MIDI::messageType_t::controlChange, descriptor.note, descriptor.velocity, descriptor.channel, true);
        }
        break;

        case messageType_t::mmcPlay:
        {
            mmcArray[4] = 0x02;
            midi.sendSysEx(6, mmcArray, true);
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::mmcPlay, mmcArray[2], 0, 0);
        }
        break;

        case messageType_t::mmcStop:
        {
            mmcArray[4] = 0x01;
            midi.sendSysEx(6, mmcArray, true);
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::mmcStop, mmcArray[2], 0, 0);
        }
        break;

        case messageType_t::mmcPause:
        {
            mmcArray[4] = 0x09;
            midi.sendSysEx(6, mmcArray, true);
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::mmcPause, mmcArray[2], 0, 0);
        }
        break;

        case messageType_t::realTimeClock:
        {
            midi.sendRealTime(MIDI::messageType_t::sysRealTimeClock);
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::sysRealTimeClock, 0, 0, 0);
        }
        break;

        case messageType_t::realTimeStart:
        {
            midi.sendRealTime(MIDI::messageType_t::sysRealTimeStart);
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::sysRealTimeStart, 0, 0, 0);
        }
        break;

        case messageType_t::realTimeContinue:
        {
            midi.sendRealTime(MIDI::messageType_t::sysRealTimeContinue);
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::sysRealTimeContinue, 0, 0, 0);
        }
        break;

        case messageType_t::realTimeStop:
        {
            midi.sendRealTime(MIDI::messageType_t::sysRealTimeStop);
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::sysRealTimeStop, 0, 0, 0);
        }
        break;

        case messageType_t::realTimeActiveSensing:
        {
            midi.sendRealTime(MIDI::messageType_t::sysRealTimeActiveSensing);
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::sysRealTimeActiveSensing, 0, 0, 0);
        }
        break;

        case messageType_t::realTimeSystemReset:
        {
            midi.sendRealTime(MIDI::messageType_t::sysRealTimeSystemReset);
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::sysRealTimeSystemReset, 0, 0, 0);
        }
        break;

        case messageType_t::mmcRecord:
        {
            //start recording
            mmcArray[4] = 0x06;
            midi.sendSysEx(6, mmcArray, true);
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::mmcRecordOn, mmcArray[2], 0, 0);
        }
        break;

        case messageType_t::multiValIncResetNote:
        {
            uint8_t currentValue = Common::currentValue(buttonID);
            uint8_t value        = Common::valueInc(buttonID, descriptor.velocity, Common::incDecType_t::reset);

            if (currentValue != value)
            {
                if (!value)
                {
                    midi.sendNoteOff(descriptor.note, value, descriptor.channel);
                    leds.midiToState(MIDI::messageType_t::noteOff, descriptor.note, value, descriptor.channel, true);
                    display.displayMIDIevent(Display::eventType_t::out, Display::event_t::noteOff, descriptor.note, value, descriptor.channel + 1);
                }
                else
                {
                    midi.sendNoteOn(descriptor.note, value, descriptor.channel);
                    leds.midiToState(MIDI::messageType_t::noteOn, descriptor.note, value, descriptor.channel, true);
                    display.displayMIDIevent(Display::eventType_t::out, Display::event_t::noteOn, descriptor.note, value, descriptor.channel + 1);
                }
            }
        }
        break;

        case messageType_t::multiValIncDecNote:
        {
            uint8_t currentValue = Common::currentValue(buttonID);
            uint8_t value        = Common::valueIncDec(buttonID, descriptor.velocity);

            if (currentValue != value)
            {
                if (!value)
                {
                    midi.sendNoteOff(descriptor.note, value, descriptor.channel);
                    leds.midiToState(MIDI::messageType_t::noteOff, descriptor.note, value, descriptor.channel, true);
                    display.displayMIDIevent(Display::eventType_t::out, Display::event_t::noteOff, descriptor.note, value, descriptor.channel + 1);
                }
                else
                {
                    midi.sendNoteOn(descriptor.note, value, descriptor.channel);
                    leds.midiToState(MIDI::messageType_t::noteOn, descriptor.note, value, descriptor.channel, true);
                    display.displayMIDIevent(Display::eventType_t::out, Display::event_t::noteOn, descriptor.note, value, descriptor.channel + 1);
                }
            }
        }
        break;

        case messageType_t::multiValIncResetCC:
        {
            uint8_t currentValue = Common::currentValue(buttonID);
            uint8_t value        = Common::valueInc(buttonID, descriptor.velocity, Common::incDecType_t::reset);

            if (currentValue != value)
            {
                midi.sendControlChange(descriptor.note, value, descriptor.channel);
                leds.midiToState(MIDI::messageType_t::controlChange, descriptor.note, value, descriptor.channel, true);
                display.displayMIDIevent(Display::eventType_t::out, Display::event_t::controlChange, descriptor.note, value, descriptor.channel + 1);
            }
        }
        break;

        case messageType_t::multiValIncDecCC:
        {
            uint8_t currentValue = Common::currentValue(buttonID);
            uint8_t value        = Common::valueIncDec(buttonID, descriptor.velocity);

            if (currentValue != value)
            {
                midi.sendControlChange(descriptor.note, value, descriptor.channel);
                leds.midiToState(MIDI::messageType_t::controlChange, descriptor.note, value, descriptor.channel, true);
                display.displayMIDIevent(Display::eventType_t::out, Display::event_t::controlChange, descriptor.note, value, descriptor.channel + 1);
            }
        }
        break;

        default:
            break;
        }
    }
    else
    {
        switch (descriptor.messageType)
        {
        case messageType_t::note:
            midi.sendNoteOff(descriptor.note, 0, descriptor.channel);
            display.displayMIDIevent(Display::eventType_t::out, midi.getNoteOffMode() == MIDI::noteOffType_t::standardNoteOff ? Display::event_t::noteOff : Display::event_t::noteOn, descriptor.note, 0, descriptor.channel + 1);
            leds.midiToState(MIDI::messageType_t::noteOff, descriptor.note, 0, descriptor.channel, true);
            break;

        case messageType_t::controlChangeReset:
            midi.sendControlChange(descriptor.note, 0, descriptor.channel);
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::controlChange, descriptor.note, 0, descriptor.channel + 1);
            leds.midiToState(MIDI::messageType_t::controlChange, descriptor.note, 0, descriptor.channel, true);
            break;

        case messageType_t::mmcRecord:
            //stop recording
            mmcArray[4] = 0x07;
            midi.sendSysEx(6, mmcArray, true);
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::mmcRecordOff, mmcArray[2], 0, 0);
            break;

        default:
            break;
        }
    }
}

///
/// \brief Updates current state of button.
/// @param [in] buttonID        Button for which state is being changed.
/// @param [in] state     New button state (true/pressed, false/released).
///
void Buttons::setButtonState(uint8_t buttonID, uint8_t state)
{
    uint8_t arrayIndex  = buttonID / 8;
    uint8_t buttonIndex = buttonID - 8 * arrayIndex;

    BIT_WRITE(buttonPressed[arrayIndex], buttonIndex, state);
}

///
/// \brief Checks for last button state.
/// @param [in] buttonID    Button index for which previous state is being checked.
/// \returns True if last state was on/pressed, false otherwise.
///
bool Buttons::getButtonState(uint8_t buttonID)
{
    uint8_t arrayIndex  = buttonID / 8;
    uint8_t buttonIndex = buttonID - 8 * arrayIndex;

    return BIT_READ(buttonPressed[arrayIndex], buttonIndex);
}

///
/// \brief Updates current state of latching button.
/// Used only for latching buttons where new state which should be sent differs
/// from last one, for instance when sending MIDI note on on first press (latching
/// state: true), and note off on second (latching state: false).
/// State should be stored in variable because unlike momentary buttons, state of
/// latching buttons doesn't necessarrily match current "real" state of button since events
/// for latching buttons are sent only on presses.
/// @param [in] buttonID    Button for which state is being changed.
/// @param [in] state       New latching state.
///
void Buttons::setLatchingState(uint8_t buttonID, uint8_t state)
{
    uint8_t arrayIndex  = buttonID / 8;
    uint8_t buttonIndex = buttonID - 8 * arrayIndex;

    BIT_WRITE(lastLatchingState[arrayIndex], buttonIndex, state);
}

///
/// \brief Checks for last latching button state.
/// @param [in] buttonID    Button index for which previous latching state is being checked.
/// \returns True if last state was on/pressed, false otherwise.
///
bool Buttons::getLatchingState(uint8_t buttonID)
{
    uint8_t arrayIndex  = buttonID / 8;
    uint8_t buttonIndex = buttonID - 8 * arrayIndex;

    return BIT_READ(lastLatchingState[arrayIndex], buttonIndex);
}

///
/// \brief Resets the current state of the specified button.
/// @param [in] buttonID    Button for which to reset state.
///
void Buttons::reset(uint8_t buttonID)
{
    setButtonState(buttonID, false);
    setLatchingState(buttonID, false);
    filter.reset(buttonID);
}