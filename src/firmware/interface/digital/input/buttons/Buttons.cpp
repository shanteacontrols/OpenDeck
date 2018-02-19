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
#include "../../../../database/Database.h"
#include "../../output/leds/LEDs.h"
#include "sysex/src/SysEx.h"
#include "../../../cinfo/CInfo.h"
#ifdef DISPLAY_SUPPORTED
#include "../../../display/Display.h"
#endif

const uint8_t   buttonDebounceCompare = 0b10000000;

static uint8_t  mmcArray[] =  { 0xF0, 0x7F, 0x7F, 0x06, 0x00, 0xF7 }; //based on MIDI spec for transport control

//variables
static uint8_t  previousButtonState[(MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG)/8+1],
                buttonPressed[(MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG)/8+1],
                buttonDebounceCounter[MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG];

Buttons::Buttons()
{
    //def const
}

void Buttons::setButtonPressed(uint8_t buttonID, bool state)
{
    uint8_t arrayIndex = buttonID/8;
    uint8_t buttonIndex = buttonID - 8*arrayIndex;

    BIT_WRITE(buttonPressed[arrayIndex], buttonIndex, state);
}

bool Buttons::getButtonPressed(uint8_t buttonID)
{
    uint8_t arrayIndex = buttonID/8;
    uint8_t buttonIndex = buttonID - 8*arrayIndex;

    return BIT_READ(buttonPressed[arrayIndex], buttonIndex);
}

void Buttons::processMomentaryButton(uint8_t buttonID, bool buttonState, buttonMIDImessage_t midiMessage)
{
    uint8_t note = database.read(DB_BLOCK_BUTTON, buttonMIDIidSection, buttonID);
    uint8_t noteChannel = database.read(DB_BLOCK_MIDI, midiChannelSection, midiChannelNote);
    uint8_t pcChannel = database.read(DB_BLOCK_MIDI, midiChannelSection, midiChannelPC);
    uint8_t ccChannel = database.read(DB_BLOCK_MIDI, midiChannelSection, midiChannelCC);
    uint8_t velocity = database.read(DB_BLOCK_BUTTON, buttonMIDIvelocitySection, buttonID);
    mmcArray[2] = note; //use midi note as channel id for transport control

    if (buttonState)
    {
        //send note on only once
        if (!getButtonPressed(buttonID))
        {
            setButtonPressed(buttonID, true);

            switch(midiMessage)
            {
                case buttonNote:
                midi.sendNoteOn(note, velocity, noteChannel);
                #ifdef DISPLAY_SUPPORTED
                display.displayMIDIevent(displayEventOut, midiMessageNoteOn_display, note, velocity, noteChannel);
                #endif
                leds.noteToState(note, velocity, true);
                break;

                case buttonPC:
                midi.sendProgramChange(note, pcChannel);
                #ifdef DISPLAY_SUPPORTED
                display.displayMIDIevent(displayEventOut, midiMessageProgramChange_display, note, 0, pcChannel);
                #endif
                break;

                case buttonCC:
                case buttonCCreset:
                midi.sendControlChange(note, velocity, ccChannel);
                #ifdef DISPLAY_SUPPORTED
                display.displayMIDIevent(displayEventOut, midiMessageControlChange_display, note, velocity, ccChannel);
                #endif
                leds.noteToState(note, velocity, true);
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

                default:
                break;
            }

            if (sysEx.configurationEnabled())
            {
                if ((rTimeMs() - getLastCinfoMsgTime(DB_BLOCK_BUTTON)) > COMPONENT_INFO_TIMEOUT)
                {
                    sysEx.startResponse();
                    sysEx.addToResponse(COMPONENT_ID_STRING);
                    sysEx.addToResponse(DB_BLOCK_BUTTON);
                    sysEx.addToResponse(buttonID);
                    sysEx.sendResponse();
                    updateCinfoTime(DB_BLOCK_BUTTON);
                }
            }
        }
    }
    else
    {
        //button is released
        if (getButtonPressed(buttonID))
        {
            switch(midiMessage)
            {
                case buttonNote:
                midi.sendNoteOff(note, 0, noteChannel);
                #ifdef DISPLAY_SUPPORTED
                display.displayMIDIevent(displayEventOut, midi.getNoteOffMode() == noteOffType_standardNoteOff ? midiMessageNoteOff_display : midiMessageNoteOn_display, note, velocity, noteChannel);
                #endif
                leds.noteToState(note, 0, true);
                break;

                case buttonCCreset:
                midi.sendControlChange(note, 0, ccChannel);
                #ifdef DISPLAY_SUPPORTED
                display.displayMIDIevent(displayEventOut, midiMessageControlChange_display, note, 0, ccChannel);
                #endif
                leds.noteToState(note, 0, true);
                break;

                default:
                break;
            }

            if (sysEx.configurationEnabled())
            {
                if ((rTimeMs() - getLastCinfoMsgTime(DB_BLOCK_BUTTON)) > COMPONENT_INFO_TIMEOUT)
                {
                    sysEx.startResponse();
                    sysEx.addToResponse(COMPONENT_ID_STRING);
                    sysEx.addToResponse(DB_BLOCK_BUTTON);
                    sysEx.addToResponse(buttonID);
                    sysEx.sendResponse();
                    updateCinfoTime(DB_BLOCK_BUTTON);
                }
            }

            setButtonPressed(buttonID, false);
        }
    }
}

void Buttons::processLatchingButton(uint8_t buttonID, bool buttonState, buttonMIDImessage_t midiMessage)
{
    if (buttonState != getPreviousButtonState(buttonID))
    {
        if (buttonState)
        {
            uint8_t note = database.read(DB_BLOCK_BUTTON, buttonMIDIidSection, buttonID);
            uint8_t noteChannel = database.read(DB_BLOCK_MIDI, midiChannelSection, midiChannelNote);
            uint8_t ccChannel = database.read(DB_BLOCK_MIDI, midiChannelSection, midiChannelCC);
            uint8_t velocity = database.read(DB_BLOCK_BUTTON, buttonMIDIvelocitySection, buttonID);
            mmcArray[2] = note;

            //button is pressed
            //if a button has been already pressed
            if (getButtonPressed(buttonID))
            {
                switch(midiMessage)
                {
                    case buttonNote:
                    midi.sendNoteOff(note, 0, noteChannel);
                    #ifdef DISPLAY_SUPPORTED
                    display.displayMIDIevent(displayEventOut, midi.getNoteOffMode() == noteOffType_standardNoteOff ? midiMessageNoteOff_display : midiMessageNoteOn_display, note, velocity, noteChannel);
                    #endif
                    leds.noteToState(note, 0, true);
                    break;

                    case buttonMMCRecord:
                    //stop recording
                    mmcArray[4] = 0x07;
                    midi.sendSysEx(6, mmcArray, true);
                    #ifdef DISPLAY_SUPPORTED
                    display.displayMIDIevent(displayEventOut, midiMessageMMCrecordOff_display, mmcArray[2], 0, 0);
                    #endif
                    break;

                    case buttonCCreset:
                    midi.sendControlChange(note, 0, ccChannel);
                    #ifdef DISPLAY_SUPPORTED
                    display.displayMIDIevent(displayEventOut, midiMessageControlChange_display, note, 0, ccChannel);
                    #endif
                    leds.noteToState(note, 0, true);
                    break;

                    default:
                    break;
                }

                if (sysEx.configurationEnabled())
                {
                    if ((rTimeMs() - getLastCinfoMsgTime(DB_BLOCK_BUTTON)) > COMPONENT_INFO_TIMEOUT)
                    {
                        sysEx.startResponse();
                        sysEx.addToResponse(COMPONENT_ID_STRING);
                        sysEx.addToResponse(DB_BLOCK_BUTTON);
                        sysEx.addToResponse(buttonID);
                        sysEx.sendResponse();
                        updateCinfoTime(DB_BLOCK_BUTTON);
                    }
                }

                //reset pressed state
                setButtonPressed(buttonID, false);
            }
            else
            {
                switch(midiMessage)
                {
                    case buttonNote:
                    midi.sendNoteOn(note, velocity, noteChannel);
                    #ifdef DISPLAY_SUPPORTED
                    display.displayMIDIevent(displayEventOut, midiMessageNoteOn_display, note, velocity, noteChannel);
                    #endif
                    leds.noteToState(note, velocity, true);
                    break;

                    case buttonMMCRecord:
                    //start recording
                    mmcArray[4] = 0x06;
                    midi.sendSysEx(6, mmcArray, true);
                    #ifdef DISPLAY_SUPPORTED
                    display.displayMIDIevent(displayEventOut, midiMessageMMCrecordOn_display, mmcArray[2], 0, 0);
                    #endif
                    break;

                    case buttonCCreset:
                    midi.sendControlChange(note, velocity, ccChannel);
                    #ifdef DISPLAY_SUPPORTED
                    display.displayMIDIevent(displayEventOut, midiMessageControlChange_display, note, velocity, ccChannel);
                    #endif
                    leds.noteToState(note, velocity, true);
                    break;

                    default:
                    break;
                }

                if (sysEx.configurationEnabled())
                {
                    if ((rTimeMs() - getLastCinfoMsgTime(DB_BLOCK_BUTTON)) > COMPONENT_INFO_TIMEOUT)
                    {
                        sysEx.startResponse();
                        sysEx.addToResponse(COMPONENT_ID_STRING);
                        sysEx.addToResponse(DB_BLOCK_BUTTON);
                        sysEx.addToResponse(buttonID);
                        sysEx.sendResponse();
                        updateCinfoTime(DB_BLOCK_BUTTON);
                    }
                }

                //toggle buttonPressed flag to true
                setButtonPressed(buttonID, true);
            }
        }
    }
}

void Buttons::processButton(uint8_t buttonID, bool state, bool debounce)
{
    bool debounced = debounce ? buttonDebounced(buttonID, state) : true;

    if (debounced)
    {
        buttonType_t type = (buttonType_t)database.read(DB_BLOCK_BUTTON, buttonTypeSection, buttonID);

        buttonMIDImessage_t midiMessage = (buttonMIDImessage_t)database.read(DB_BLOCK_BUTTON, buttonMIDImessageSection, buttonID);

        //overwrite type under certain conditions
        switch(midiMessage)
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

        switch (type)
        {
            case buttonMomentary:
            processMomentaryButton(buttonID, state, midiMessage);
            break;

            case buttonLatching:
            processLatchingButton(buttonID, state, midiMessage);
            break;

            default:
            break;
        }

        updateButtonState(buttonID, state);
    }
}

void Buttons::update()
{
    for (int i=0; i<MAX_NUMBER_OF_BUTTONS; i++)
    {
        bool buttonState;
        uint8_t encoderPairIndex = board.getEncoderPair(i);

        if (database.read(DB_BLOCK_ENCODER, encoderEnabledSection, encoderPairIndex))
            buttonState = false;    //button is member of encoder pair, always set state to released
        else
            buttonState = board.getButtonState(i);

        processButton(i, buttonState);
    }
}

void Buttons::updateButtonState(uint8_t buttonID, uint8_t buttonState)
{
    uint8_t arrayIndex = buttonID/8;
    uint8_t buttonIndex = buttonID - 8*arrayIndex;

    //update state if it's different than last one
    if (BIT_READ(previousButtonState[arrayIndex], buttonIndex) != buttonState)
        BIT_WRITE(previousButtonState[arrayIndex], buttonIndex, buttonState);
}

bool Buttons::getPreviousButtonState(uint8_t buttonID)
{
    uint8_t arrayIndex = buttonID/8;
    uint8_t buttonIndex = buttonID - 8*arrayIndex;

    return BIT_READ(previousButtonState[arrayIndex], buttonIndex);
}

bool Buttons::buttonDebounced(uint8_t buttonID, bool buttonState)
{
    //shift new button reading into previousButtonState
    buttonDebounceCounter[buttonID] = (buttonDebounceCounter[buttonID] << (uint8_t)1) | (uint8_t)buttonState | buttonDebounceCompare;

    //if button is debounced, return true
    return ((buttonDebounceCounter[buttonID] == buttonDebounceCompare) || (buttonDebounceCounter[buttonID] == 0xFF));
}

Buttons buttons;
