/*

Copyright 2015-2019 Igor Petrovic

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
#include "board/Board.h"
#include "core/src/general/BitManipulation.h"

using namespace Interface::digital::input;

///
/// \brief Continuously reads inputs from buttons and acts if necessary.
///
void Buttons::update()
{
    for (int i=0; i<MAX_NUMBER_OF_BUTTONS; i++)
    {
        if (database.read(DB_BLOCK_ENCODERS, dbSection_encoders_enable, Board::interface::digital::input::getEncoderPair(i)))
            continue;

        bool state = Board::interface::digital::input::getButtonState(i);

        if (!buttonDebounced(i, state))
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
    //act on change of state only
    if (state == getButtonState(buttonID))
        return;

    setButtonState(buttonID, state);

    messageType_t buttonMessage = static_cast<messageType_t>(database.read(DB_BLOCK_BUTTONS, dbSection_buttons_midiMessage, buttonID));

    //don't process messageType_t::none type of message
    if (buttonMessage != messageType_t::none)
    {
        type_t type = static_cast<type_t>(database.read(DB_BLOCK_BUTTONS, dbSection_buttons_type, buttonID));

        bool sendMIDI = true;

        //overwrite type under certain conditions
        switch(buttonMessage)
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
            type = type_t::momentary;
            break;

            case messageType_t::mmcRecord:
            type = type_t::latching;
            break;

            case messageType_t::presetOpenDeck:
            type = type_t::momentary;
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
            sendMessage(buttonID, state, buttonMessage);
        }
        else if (buttonMessage == messageType_t::presetOpenDeck)
        {
            uint8_t preset = database.read(DB_BLOCK_BUTTONS, dbSection_buttons_midiID, buttonID);
            database.setPreset(preset);
        }
        else if (buttonMessage == messageType_t::customHook)
        {
            customHook(buttonID, state);
        }
    }

    cInfo.send(DB_BLOCK_BUTTONS, buttonID);
}

///
/// \brief Used to send MIDI message from specified button.
/// Used internally once the button state has been changed and processed.
/// @param [in] buttonID        Button ID which sends the message.
/// @param [in] state           Button state (true/pressed, false/released).
/// @param [in] buttonMessage   Type of MIDI message to send. If unspecified, message type is read from database.
///
void Buttons::sendMessage(uint8_t buttonID, bool state, messageType_t buttonMessage)
{
    uint8_t note = database.read(DB_BLOCK_BUTTONS, dbSection_buttons_midiID, buttonID);
    uint8_t channel = database.read(DB_BLOCK_BUTTONS, dbSection_buttons_midiChannel, buttonID);
    uint8_t velocity = database.read(DB_BLOCK_BUTTONS, dbSection_buttons_velocity, buttonID);

    if (buttonMessage == messageType_t::AMOUNT)
        buttonMessage = static_cast<messageType_t>(database.read(DB_BLOCK_BUTTONS, dbSection_buttons_midiMessage, buttonID));

    mmcArray[2] = note; //use midi note as channel id for transport control

    if (state)
    {
        switch(buttonMessage)
        {
            case messageType_t::note:
            midi.sendNoteOn(note, velocity, channel);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::noteOn, note, velocity, channel+1);
            #endif
            #ifdef LEDS_SUPPORTED
            leds.midiToState(MIDI::messageType_t::noteOn, note, velocity, channel, true);
            #endif
            break;

            case messageType_t::programChange:
            case messageType_t::programChangeInc:
            case messageType_t::programChangeDec:
            if (buttonMessage != messageType_t::programChange)
            {
                if (buttonMessage == messageType_t::programChangeInc)
                {
                    if (Common::lastPCvalue[channel] < 127)
                        Common::lastPCvalue[channel]++;
                }
                else
                {
                    if (Common::lastPCvalue[channel] > 0)
                        Common::lastPCvalue[channel]--;
                }

                note = Common::lastPCvalue[channel];
            }

            midi.sendProgramChange(note, channel);
            #ifdef LEDS_SUPPORTED
            leds.midiToState(MIDI::messageType_t::programChange, note, 0, channel, true);
            #endif
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::programChange, note, 0, channel+1);
            #endif
            break;

            case messageType_t::controlChange:
            case messageType_t::controlChangeReset:
            midi.sendControlChange(note, velocity, channel);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::controlChange, note, velocity, channel+1);
            #endif
            #ifdef LEDS_SUPPORTED
            leds.midiToState(MIDI::messageType_t::controlChange, note, velocity, channel, true);
            #endif
            break;

            case messageType_t::mmcPlay:
            mmcArray[4] = 0x02;
            midi.sendSysEx(6, mmcArray, true);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::mmcPlay, mmcArray[2], 0, 0);
            #endif
            break;

            case messageType_t::mmcStop:
            mmcArray[4] = 0x01;
            midi.sendSysEx(6, mmcArray, true);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::mmcStop, mmcArray[2], 0, 0);
            #endif
            break;

            case messageType_t::mmcPause:
            mmcArray[4] = 0x09;
            midi.sendSysEx(6, mmcArray, true);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::mmcPause, mmcArray[2], 0, 0);
            #endif
            break;

            case messageType_t::realTimeClock:
            midi.sendRealTime(MIDI::messageType_t::sysRealTimeClock);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::sysRealTimeClock, 0, 0, 0);
            #endif
            break;

            case messageType_t::realTimeStart:
            midi.sendRealTime(MIDI::messageType_t::sysRealTimeStart);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::sysRealTimeStart, 0, 0, 0);
            #endif
            break;

            case messageType_t::realTimeContinue:
            midi.sendRealTime(MIDI::messageType_t::sysRealTimeContinue);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::sysRealTimeContinue, 0, 0, 0);
            #endif
            break;

            case messageType_t::realTimeStop:
            midi.sendRealTime(MIDI::messageType_t::sysRealTimeStop);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::sysRealTimeStop, 0, 0, 0);
            #endif
            break;

            case messageType_t::realTimeActiveSensing:
            midi.sendRealTime(MIDI::messageType_t::sysRealTimeActiveSensing);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::sysRealTimeActiveSensing, 0, 0, 0);
            #endif
            break;

            case messageType_t::realTimeSystemReset:
            midi.sendRealTime(MIDI::messageType_t::sysRealTimeSystemReset);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::sysRealTimeSystemReset, 0, 0, 0);
            #endif
            break;

            case messageType_t::mmcRecord:
            //start recording
            mmcArray[4] = 0x06;
            midi.sendSysEx(6, mmcArray, true);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::mmcRecordOn, mmcArray[2], 0, 0);
            #endif
            break;

            default:
            break;
        }
    }
    else
    {
        switch(buttonMessage)
        {
            case messageType_t::note:
            midi.sendNoteOff(note, 0, channel);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(Display::eventType_t::out, midi.getNoteOffMode() == MIDI::noteOffType_t::standardNoteOff ? Display::event_t::noteOff : Display::event_t::noteOn, note, 0, channel+1);
            #endif
            #ifdef LEDS_SUPPORTED
            leds.midiToState(MIDI::messageType_t::noteOff, note, 0, channel, true);
            #endif
            break;

            case messageType_t::controlChangeReset:
            midi.sendControlChange(note, 0, channel);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::controlChange, note, 0, channel+1);
            #endif
            #ifdef LEDS_SUPPORTED
            leds.midiToState(MIDI::messageType_t::controlChange, 0, channel, true);
            #endif
            break;

            case messageType_t::mmcRecord:
            //stop recording
            mmcArray[4] = 0x07;
            midi.sendSysEx(6, mmcArray, true);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(Display::eventType_t::out, Display::event_t::mmcRecordOff, mmcArray[2], 0, 0);
            #endif
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
    uint8_t arrayIndex = buttonID/8;
    uint8_t buttonIndex = buttonID - 8*arrayIndex;

    BIT_WRITE(buttonPressed[arrayIndex], buttonIndex, state);
}

///
/// \brief Checks for last button state.
/// @param [in] buttonID    Button index for which previous state is being checked.
/// \returns True if last state was on/pressed, false otherwise.
///
bool Buttons::getButtonState(uint8_t buttonID)
{
    uint8_t arrayIndex = buttonID/8;
    uint8_t buttonIndex = buttonID - 8*arrayIndex;

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
    uint8_t arrayIndex = buttonID/8;
    uint8_t buttonIndex = buttonID - 8*arrayIndex;

    BIT_WRITE(lastLatchingState[arrayIndex], buttonIndex, state);
}

///
/// \brief Checks for last latching button state.
/// @param [in] buttonID    Button index for which previous latching state is being checked.
/// \returns True if last state was on/pressed, false otherwise.
///
bool Buttons::getLatchingState(uint8_t buttonID)
{
    uint8_t arrayIndex = buttonID/8;
    uint8_t buttonIndex = buttonID - 8*arrayIndex;

    return BIT_READ(lastLatchingState[arrayIndex], buttonIndex);
}

///
/// \brief Checks if button reading is stable.
/// Shift old value to the left, append new value and
/// append DEBOUNCE_COMPARE with OR command. If final value is equal to 0xFF or
/// DEBOUNCE_COMPARE, signal is debounced.
/// @param [in] buttonID    Button index which is being checked.
/// @param [in] state Current button state.
/// \returns                True if button reading is stable, false otherwise.
///
bool Buttons::buttonDebounced(uint8_t buttonID, bool state)
{
    //shift new button reading into previousButtonState
    buttonDebounceCounter[buttonID] = (buttonDebounceCounter[buttonID] << (uint8_t)1) | (uint8_t)state | BUTTON_DEBOUNCE_COMPARE;

    //if button is debounced, return true
    return ((buttonDebounceCounter[buttonID] == BUTTON_DEBOUNCE_COMPARE) || (buttonDebounceCounter[buttonID] == 0xFF));
}

bool Buttons::getStateFromAnalogValue(uint16_t adcValue)
{
    //button pressed
    //set state to released only if value is below ADC_DIGITAL_VALUE_THRESHOLD_OFF
    if (adcValue < ADC_DIGITAL_VALUE_THRESHOLD_OFF)
        return false;
    else if (adcValue > ADC_DIGITAL_VALUE_THRESHOLD_ON)
        return true;
    else
        return false;
}