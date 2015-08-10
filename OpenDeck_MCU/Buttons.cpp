/*

OpenDECK library v1.3
File: Buttons.cpp
Last revision date: 2014-12-25
Author: Igor Petrovic

*/

#include "OpenDeck.h"

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

    if (!boardObject.digitalInDataAvailable()) return;

    uint8_t columnState = boardObject.getDigitalInData();
    uint8_t columnNumber = boardObject.getActiveColumn();

    if (columnStable(columnState, columnNumber))    {

        for (int i=0; i<NUMBER_OF_BUTTON_ROWS; i++)   {

            //extract current bit from çolumnState variable
            //invert extracted bit because of pull-up resistors
            uint8_t buttonState = !((columnState >> i) & 0x01);
            //get current button number based on row and column
            uint8_t buttonNumber = columnNumber+i*NUMBER_OF_BUTTON_COLUMNS;

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

    lastColumnState[columnNumber] = columnState;

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

bool OpenDeck::columnStable(uint16_t columnState, uint8_t columnNumber)   {

    //column reading is declared stable if there are numberOfColumnPasses same readings
    if (columnState == lastColumnState[columnNumber])   {

        if (columnPassCounter[columnNumber] < boardObject.getNumberOfColumnPasses()) columnPassCounter[columnNumber]++;

    }   else columnPassCounter[columnNumber] = 0;

    //iterate over rows if column readings are stable
    return (columnPassCounter[columnNumber] == boardObject.getNumberOfColumnPasses());

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