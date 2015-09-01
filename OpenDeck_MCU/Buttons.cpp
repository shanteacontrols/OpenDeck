/*

OpenDECK library v1.3
File: Buttons.cpp
Last revision date: 2014-12-25
Author: Igor Petrovic

*/

#include "OpenDeck.h"

const uint8_t buttonDebounceCompare = 0b11111000;

void OpenDeck::setButtonPressed(uint8_t buttonNumber, bool state)   {

    uint8_t arrayIndex = buttonNumber/8;
    uint8_t buttonIndex = buttonNumber - 8*arrayIndex;

    bitWrite(buttonPressed[arrayIndex], buttonIndex, state);

}

buttonType OpenDeck::getButtonType(uint8_t buttonNumber)  {

    uint8_t arrayIndex = buttonNumber/8;
    uint8_t buttonIndex = buttonNumber - 8*arrayIndex;

    if (bitRead(_buttonType[arrayIndex], buttonIndex))
        return buttonLatching;

    return buttonMomentary;

}

bool OpenDeck::getButtonPCenabled(uint8_t buttonNumber)   {

    uint8_t arrayIndex = buttonNumber/8;
    uint8_t buttonIndex = buttonNumber - 8*arrayIndex;

    return bitRead(buttonPCenabled[arrayIndex], buttonIndex);

}

uint8_t OpenDeck::getButtonNote(uint8_t buttonNumber)   {

    return noteNumber[buttonNumber];

}

bool OpenDeck::getButtonPressed(uint8_t buttonNumber)   {

    uint8_t arrayIndex = buttonNumber/8;
    uint8_t buttonIndex = buttonNumber - 8*arrayIndex;

    return bitRead(buttonPressed[arrayIndex], buttonIndex);

}

void OpenDeck::processMomentaryButton(uint8_t buttonNumber, bool buttonState)   {

    if (buttonState)    {

        //send note on only once
        if (!getButtonPressed(buttonNumber))    {

            setButtonPressed(buttonNumber, true);

            if (getButtonPCenabled(buttonNumber))    {

                sendProgramChange(_programChangeChannel, noteNumber[buttonNumber]);
                return;

            }

            sendMIDInote(noteNumber[buttonNumber], true, _noteChannel);

        }

    }   else {  //button is released

            if (getButtonPressed(buttonNumber))    {

                if (!getButtonPCenabled(buttonNumber))
                    sendMIDInote(noteNumber[buttonNumber], false, _noteChannel);

                setButtonPressed(buttonNumber, false);

            }

        }

}

void OpenDeck::processLatchingButton(uint8_t buttonNumber, bool buttonState)    {

    if (buttonState != getPreviousButtonState(buttonNumber)) {

        if (buttonState) {

            //button is pressed
            //if a button has been already pressed
            if (getButtonPressed(buttonNumber)) {

                sendMIDInote(noteNumber[buttonNumber], false, _noteChannel);

                //reset pressed state
                setButtonPressed(buttonNumber, false);

                } else {

                //send note on
                sendMIDInote(noteNumber[buttonNumber], true, _noteChannel);

                //toggle buttonPressed flag to true
                setButtonPressed(buttonNumber, true);

            }

        }

    }

}

void OpenDeck::readButtons()    {

    #ifdef BOARD

    uint8_t availableButtonData = boardObject.buttonDataAvailable();
    if (!availableButtonData) return;

        for (int i=0; i<availableButtonData; i++)   {

            uint8_t buttonNumber = boardObject.getButtonNumber(i);
            uint8_t buttonState = boardObject.getButtonState(i);

            if (buttonDebounced(buttonNumber, buttonState))  {

                switch (getButtonType(buttonNumber))   {

                    case buttonLatching:
                    processLatchingButton(buttonNumber, buttonState);
                    break;

                    case buttonMomentary:
                    processMomentaryButton(buttonNumber, buttonState);
                    break;

                    default:
                    break;

                }

                updateButtonState(buttonNumber, buttonState);

            }

        }

    #endif

}

void OpenDeck::updateButtonState(uint8_t buttonNumber, uint8_t buttonState) {

    uint8_t arrayIndex = buttonNumber/8;
    uint8_t buttonIndex = buttonNumber - 8*arrayIndex;

    //update state if it's different than last one
    if (bitRead(previousButtonState[arrayIndex], buttonIndex) != buttonState)
        bitWrite(previousButtonState[arrayIndex], buttonIndex, buttonState);

}

bool OpenDeck::getPreviousButtonState(uint8_t buttonNumber) {

    uint8_t arrayIndex = buttonNumber/8;
    uint8_t buttonIndex = buttonNumber - 8*arrayIndex;

    return bitRead(previousButtonState[arrayIndex], buttonIndex);

}

bool OpenDeck::buttonDebounced(uint8_t buttonNumber, bool buttonState)   {

    //shift new button reading into previousButtonState
    buttonDebounceCounter[buttonNumber] = (buttonDebounceCounter[buttonNumber] << 1) | buttonState | buttonDebounceCompare;

    //if button is debounced, return true
    return ((buttonDebounceCounter[buttonNumber] == buttonDebounceCompare) || (buttonDebounceCounter[buttonNumber] == 0xFF));

}

void OpenDeck::sendMIDInote(uint8_t buttonNote, bool buttonState, uint8_t channel)  {

    switch (buttonState) {

        case false:
        //button released
        if (standardNoteOffEnabled())   {

            #ifdef USBMIDI
            usbMIDI.sendNoteOff(buttonNote, velocityOff, channel);
            #endif

            #ifdef HW_MIDI
                MIDI.sendNoteOff(buttonNote, velocityOff, channel);
            #endif

        } else {

            #ifdef HW_MIDI
                MIDI.sendNoteOn(buttonNote, velocityOff, channel);
            #endif

            #ifdef USBMIDI
                usbMIDI.sendNoteOn(buttonNote, velocityOff, channel);
            #endif

        }
        break;

        case true:
        //button pressed
        #ifdef HW_MIDI
            MIDI.sendNoteOn(buttonNote, velocityOn, channel);
        #endif

        #ifdef USBMIDI
            usbMIDI.sendNoteOn(buttonNote, velocityOn, channel);
        #endif
        break;

    }

}

void OpenDeck::sendProgramChange(uint8_t channel, uint8_t program)    {

    #ifdef USBMIDI
        usbMIDI.sendProgramChange(program, channel);
    #endif

    #ifdef HW_MIDI
        MIDI.sendProgramChange(program, channel);
    #endif

}