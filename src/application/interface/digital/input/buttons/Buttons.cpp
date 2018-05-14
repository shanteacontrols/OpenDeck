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
#include "board/Board.h"
#include "database/Database.h"
#include "interface/digital/output/leds/LEDs.h"
#include "sysex/src/SysEx.h"
#include "interface/cinfo/CInfo.h"
#ifdef DISPLAY_SUPPORTED
#include "interface/display/Display.h"
#endif

///
/// \brief Array used for simpler building of transport control messages.
/// Based on MIDI specification for transport control.
///
static uint8_t  mmcArray[] =  { 0xF0, 0x7F, 0x7F, 0x06, 0x00, 0xF7 };

///
/// \brief Array holding debounce count for all buttons to avoid incorrect state detection.
///
uint8_t     buttonDebounceCounter[MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG];

///
/// \brief Array holding current state for all buttons.
///
uint8_t     buttonPressed[(MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG)/8+1];

///
/// \brief Array holding last sent state for latching buttons only.
///
uint8_t     lastLatchingState[(MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG)/8+1];

///
/// \brief Default constructor.
///
Buttons::Buttons()
{

}

///
/// \brief Continuously reads inputs from buttons and acts if necessary.
///
void Buttons::update()
{
    for (int i=0; i<MAX_NUMBER_OF_BUTTONS; i++)
    {
        bool state = board.getButtonState(i);
        uint8_t encoderPairIndex = board.getEncoderPair(i);

        if (buttonDebounced(i, state) && !database.read(DB_BLOCK_ENCODERS, dbSection_encoders_enable, encoderPairIndex))
        {
            bool process = false;
            buttonType_t type = (buttonType_t)database.read(DB_BLOCK_BUTTONS, dbSection_buttons_type, i);

            //act on change of state only
            if (state != getButtonState(i))
            {
                setButtonState(i, state);

                //overwrite type under certain conditions
                switch((buttonMIDImessage_t)database.read(DB_BLOCK_BUTTONS, dbSection_buttons_midiMessage, i))
                {
                    case buttonPC:
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

                    default:
                    break;
                }

                switch(type)
                {
                    case buttonMomentary:
                    //always process momentary buttons
                    process = true;
                    break;

                    case buttonLatching:
                    //act on press only
                    if (state)
                    {
                        process = true;

                        if (getLatchingState(i))
                        {
                            setLatchingState(i, false);
                            //overwrite before processing
                            state = false;
                        }
                        else
                        {
                            setLatchingState(i, true);
                            state = true;
                        }
                    }
                    break;

                    default:
                    break;
                }
            }

            if (process)
                processButton(i, state);
        }
    }
}

///
/// \brief Handles changes in button states.
/// @param [in] buttonID    Button index which has changed state.
/// @param [in] state       Current button state.
///
void Buttons::processButton(uint8_t buttonID, bool state)
{
    uint8_t note = database.read(DB_BLOCK_BUTTONS, dbSection_buttons_midiID, buttonID);
    uint8_t channel = database.read(DB_BLOCK_BUTTONS, dbSection_buttons_midiChannel, buttonID);
    uint8_t velocity = database.read(DB_BLOCK_BUTTONS, dbSection_buttons_velocity, buttonID);
    buttonMIDImessage_t midiMessage = (buttonMIDImessage_t)database.read(DB_BLOCK_BUTTONS, dbSection_buttons_midiMessage, buttonID);
    mmcArray[2] = note; //use midi note as channel id for transport control

    if (state)
    {
        switch(midiMessage)
        {
            case buttonNote:
            midi.sendNoteOn(note, velocity, channel);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageNoteOn_display, note, velocity, channel+1);
            #endif
            leds.noteToState(note, velocity, 0, true);
            break;

            case buttonPC:
            midi.sendProgramChange(note, channel);
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
            leds.noteToState(note, velocity, 0, true);
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
        switch(midiMessage)
        {
            case buttonNote:
            midi.sendNoteOff(note, 0, channel);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midi.getNoteOffMode() == noteOffType_standardNoteOff ? midiMessageNoteOff_display : midiMessageNoteOn_display, note, 0, channel+1);
            #endif
            leds.noteToState(note, 0, 0, true);
            break;

            case buttonCCreset:
            midi.sendControlChange(note, 0, channel);
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventOut, midiMessageControlChange_display, note, 0, channel+1);
            #endif
            leds.noteToState(note, 0, 0, true);
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

    if (sysEx.isConfigurationEnabled())
    {
        if ((rTimeMs() - getLastCinfoMsgTime(DB_BLOCK_BUTTONS)) > COMPONENT_INFO_TIMEOUT)
        {
            sysExParameter_t cInfoMessage[] =
            {
                COMPONENT_ID_STRING,
                DB_BLOCK_BUTTONS,
                (sysExParameter_t)buttonID
            };

            sysEx.sendCustomMessage(usbMessage.sysexArray, cInfoMessage, 3);
            updateCinfoTime(DB_BLOCK_BUTTONS);
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
/// @param [in] buttonID        Button for which state is being changed.
/// @param [in] state     New latching state.
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

Buttons buttons;
