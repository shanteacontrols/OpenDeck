/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2017 Igor Petrovic

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

const uint8_t buttonDebounceCompare = 0b10000000;

uint8_t mmcArray[] =  { 0xF0, 0x7F, 0x7F, 0x06, 0x00, 0xF7 }; //based on MIDI spec for transport control

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
                midi.sendNoteOn(note, velocity, database.read(DB_BLOCK_MIDI, midiChannelSection, noteChannel));
                leds.noteToState(note, velocity, true);
                break;

                case buttonPC:
                midi.sendProgramChange(note, database.read(DB_BLOCK_MIDI, midiChannelSection, programChangeChannel));
                break;

                case buttonCC:
                midi.sendControlChange(note, velocity, database.read(DB_BLOCK_MIDI, midiChannelSection, noteChannel));
                break;

                case buttonMMCPlay:
                mmcArray[4] = 0x02;
                midi.sendSysEx(6, mmcArray, true);
                break;

                case buttonMMCStop:
                mmcArray[4] = 0x01;
                midi.sendSysEx(6, mmcArray, true);
                break;

                case buttonMMCPause:
                mmcArray[4] = 0x09;
                midi.sendSysEx(6, mmcArray, true);
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
                midi.sendNoteOff(note, 0, database.read(DB_BLOCK_MIDI, midiChannelSection, noteChannel));
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
            uint8_t channel = database.read(DB_BLOCK_MIDI, midiChannelSection, noteChannel);
            uint8_t velocity = database.read(DB_BLOCK_BUTTON, buttonMIDIvelocitySection, buttonID);
            mmcArray[2] = note;

            //button is pressed
            //if a button has been already pressed
            if (getButtonPressed(buttonID))
            {
                switch(midiMessage)
                {
                    case buttonNote:
                    midi.sendNoteOff(note, 0, channel);
                    leds.noteToState(note, 0, true);
                    break;

                    case buttonMMCRecord:
                    //stop recording
                    mmcArray[4] = 0x07;
                    midi.sendSysEx(6, mmcArray, true);
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
                    midi.sendNoteOn(note, velocity, channel);
                    leds.noteToState(note, velocity, true);
                    break;

                    case buttonMMCRecord:
                    //start recording
                    mmcArray[4] = 0x06;
                    midi.sendSysEx(6, mmcArray, true);
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
            case buttonCC:
            case buttonMMCPlay:
            case buttonMMCStop:
            case buttonMMCPause:
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
    buttonDebounceCounter[buttonID] = (buttonDebounceCounter[buttonID] << 1) | buttonState | buttonDebounceCompare;

    //if button is debounced, return true
    return ((buttonDebounceCounter[buttonID] == buttonDebounceCompare) || (buttonDebounceCounter[buttonID] == 0xFF));
}

Buttons buttons;
