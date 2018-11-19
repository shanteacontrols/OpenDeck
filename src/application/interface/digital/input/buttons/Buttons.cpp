/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Buttons.h"
#include "interface/digital/input/Common.h"
#include "core/src/general/BitManipulation.h"
#include "interface/CInfo.h"

///
/// \brief Continuously reads inputs from buttons and acts if necessary.
///
void Buttons::update()
{
    for (int i=0; i<MAX_NUMBER_OF_BUTTONS; i++)
    {
        if (database.read(DB_BLOCK_ENCODERS, dbSection_encoders_enable, Board::getEncoderPair(i)))
            continue;

        bool state = Board::getButtonState(i);

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

    buttonMIDImessage_t buttonMessage = static_cast<buttonMIDImessage_t>(database.read(DB_BLOCK_BUTTONS, dbSection_buttons_midiMessage, buttonID));

    //don't process buttonNone type of message
    if (buttonMessage != buttonNone)
    {
        buttonType_t type = static_cast<buttonType_t>(database.read(DB_BLOCK_BUTTONS, dbSection_buttons_type, buttonID));

        bool sendMIDI = true;

        //overwrite type under certain conditions
        switch(buttonMessage)
        {
            case buttonPC:
            case buttonPCinc:
            case buttonPCdec:
            case buttonMMCPlay:
            case buttonMMCStop:
            case buttonMMCPause:
            case buttonCC:
            case buttonRealTimeClock:
            case buttonRealTimeStart:
            case buttonRealTimeContinue:
            case buttonRealTimeStop:
            case buttonRealTimeActiveSensing:
            case buttonRealTimeSystemReset:
            type = buttonMomentary;
            break;

            case buttonMMCRecord:
            type = buttonLatching;
            break;

            case buttonChangePreset:
            type = buttonMomentary;
            sendMIDI = false;
            break;

            default:
            break;
        }

        if (type == buttonLatching)
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
        else
        {
            if (buttonMessage == buttonChangePreset)
            {
                uint8_t preset = database.read(DB_BLOCK_BUTTONS, dbSection_buttons_midiID, buttonID);

                #ifdef LEDS_SUPPORTED
                if (database.setPreset(preset))
                {
                    leds.midiToState(midiMessageProgramChange, preset, 0, 0, true);

                    #ifdef DISPLAY_SUPPORTED
                    display.displayMIDIevent(displayEventIn, messagePresetChange_display, preset, 0, 0);
                    #endif
                }
                #endif
            }
        }
    }

    if (cinfoHandler != nullptr)
        (*cinfoHandler)(DB_BLOCK_BUTTONS, buttonID);
}

///
/// \brief Used to send MIDI message from specified button.
/// Used internally once the button state has been changed and processed.
/// @param [in] buttonID        Button ID which sends the message.
/// @param [in] state           Button state (true/pressed, false/released).
/// @param [in] buttonMessage   Type of MIDI message to send. If unspecified, message type is read from database.
///
void Buttons::sendMessage(uint8_t buttonID, bool state, buttonMIDImessage_t buttonMessage)
{
    uint8_t note = database.read(DB_BLOCK_BUTTONS, dbSection_buttons_midiID, buttonID);
    uint8_t channel = database.read(DB_BLOCK_BUTTONS, dbSection_buttons_midiChannel, buttonID);
    uint8_t velocity = database.read(DB_BLOCK_BUTTONS, dbSection_buttons_velocity, buttonID);

    if (buttonMessage == BUTTON_MESSAGE_TYPES)
        buttonMessage = static_cast<buttonMIDImessage_t>(database.read(DB_BLOCK_BUTTONS, dbSection_buttons_midiMessage, buttonID));

    mmcArray[2] = note; //use midi note as channel id for transport control

    if (state)
    {
        switch(buttonMessage)
        {
            case buttonNote:
            midi.sendNoteOn(note, velocity, channel);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageNoteOn_display, note, velocity, channel+1);
            #endif
            #ifdef LEDS_SUPPORTED
            leds.midiToState(midiMessageNoteOn, note, velocity, channel, true);
            #endif
            break;

            case buttonPC:
            case buttonPCinc:
            case buttonPCdec:
            if (buttonMessage != buttonPC)
            {
                if (buttonMessage == buttonPCinc)
                {
                    if (digitalInputCommon::detail::lastPCvalue[channel] < 127)
                        digitalInputCommon::detail::lastPCvalue[channel]++;
                }
                else
                {
                    if (digitalInputCommon::detail::lastPCvalue[channel] > 0)
                        digitalInputCommon::detail::lastPCvalue[channel]--;
                }

                note = digitalInputCommon::detail::lastPCvalue[channel];
            }

            midi.sendProgramChange(note, channel);
            #ifdef LEDS_SUPPORTED
            leds.midiToState(midiMessageProgramChange, note, 0, channel, true);
            #endif
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageProgramChange_display, note, 0, channel+1);
            #endif
            break;

            case buttonCC:
            case buttonCCreset:
            midi.sendControlChange(note, velocity, channel);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageControlChange_display, note, velocity, channel+1);
            #endif
            #ifdef LEDS_SUPPORTED
            leds.midiToState(midiMessageControlChange, note, velocity, channel, true);
            #endif
            break;

            case buttonMMCPlay:
            mmcArray[4] = 0x02;
            midi.sendSysEx(6, mmcArray, true);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageMMCplay_display, mmcArray[2], 0, 0);
            #endif
            break;

            case buttonMMCStop:
            mmcArray[4] = 0x01;
            midi.sendSysEx(6, mmcArray, true);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageMMCstop_display, mmcArray[2], 0, 0);
            #endif
            break;

            case buttonMMCPause:
            mmcArray[4] = 0x09;
            midi.sendSysEx(6, mmcArray, true);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageMMCpause_display, mmcArray[2], 0, 0);
            #endif
            break;

            case buttonRealTimeClock:
            midi.sendRealTime(midiMessageClock);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageClock_display, 0, 0, 0);
            #endif
            break;

            case buttonRealTimeStart:
            midi.sendRealTime(midiMessageStart);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageStart_display, 0, 0, 0);
            #endif
            break;

            case buttonRealTimeContinue:
            midi.sendRealTime(midiMessageContinue);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageContinue_display, 0, 0, 0);
            #endif
            break;

            case buttonRealTimeStop:
            midi.sendRealTime(midiMessageStop);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageStop_display, 0, 0, 0);
            #endif
            break;

            case buttonRealTimeActiveSensing:
            midi.sendRealTime(midiMessageActiveSensing);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageActiveSensing_display, 0, 0, 0);
            #endif
            break;

            case buttonRealTimeSystemReset:
            midi.sendRealTime(midiMessageSystemReset);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageSystemReset_display, 0, 0, 0);
            #endif
            break;

            case buttonMMCRecord:
            //start recording
            mmcArray[4] = 0x06;
            midi.sendSysEx(6, mmcArray, true);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageMMCrecordOn_display, mmcArray[2], 0, 0);
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
            case buttonNote:
            midi.sendNoteOff(note, 0, channel);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midi.getNoteOffMode() == noteOffType_standardNoteOff ? midiMessageNoteOff_display : midiMessageNoteOn_display, note, 0, channel+1);
            #endif
            #ifdef LEDS_SUPPORTED
            leds.midiToState(midiMessageNoteOff, note, 0, channel, true);
            #endif
            break;

            case buttonCCreset:
            midi.sendControlChange(note, 0, channel);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageControlChange_display, note, 0, channel+1);
            #endif
            #ifdef LEDS_SUPPORTED
            leds.midiToState(midiMessageControlChange, 0, channel, true);
            #endif
            break;

            case buttonMMCRecord:
            //stop recording
            mmcArray[4] = 0x07;
            midi.sendSysEx(6, mmcArray, true);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageMMCrecordOff_display, mmcArray[2], 0, 0);
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
    buttonPressed[buttonID] = state;
}

///
/// \brief Checks for last button state.
/// @param [in] buttonID    Button index for which previous state is being checked.
/// \returns True if last state was on/pressed, false otherwise.
///
bool Buttons::getButtonState(uint8_t buttonID)
{
    return buttonPressed[buttonID];
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
    lastLatchingState[buttonID] = state;
}

///
/// \brief Checks for last latching button state.
/// @param [in] buttonID    Button index for which previous latching state is being checked.
/// \returns True if last state was on/pressed, false otherwise.
///
bool Buttons::getLatchingState(uint8_t buttonID)
{
    return lastLatchingState[buttonID];
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