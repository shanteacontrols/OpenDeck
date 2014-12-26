/*

OpenDECK library v1.3
File: Buttons.cpp
Last revision date: 2014-12-25
Author: Igor Petrovic

*/

#include "OpenDeck.h"
#include "Ownduino.h"

#define MIN_BUTTON_DEBOUNCE_TIME            20


uint8_t OpenDeck::getRowPassTime() {

    return COLUMN_SCAN_TIME*_numberOfColumns;

}

void OpenDeck::setNumberOfColumnPasses() {

    /*

        Algorithm calculates how many times does it need to read whole row
        before it can declare button reading stable.

    */

    uint8_t rowPassTime = getRowPassTime();
    uint8_t mod = 0;

    if ((MIN_BUTTON_DEBOUNCE_TIME % rowPassTime) > 0)   mod = 1;

    numberOfColumnPasses = ((MIN_BUTTON_DEBOUNCE_TIME / rowPassTime) + mod);

    setNumberOfLongPressPasses();

}

void OpenDeck::setNumberOfLongPressPasses() {

    longPressColumnPass = sysExGetButtonHwParameter(SYS_EX_BUTTON_HW_P_LONG_PRESS_TIME)*100 / getRowPassTime();

}

void OpenDeck::setButtonPressed(uint8_t buttonNumber, bool state)   {

    uint8_t arrayIndex = buttonNumber/8;
    uint8_t buttonIndex = buttonNumber - 8*arrayIndex;

    bitWrite(buttonPressed[arrayIndex], buttonIndex, state);

}

void OpenDeck::setButtonLongPressed(uint8_t buttonNumber, bool state)   {

    uint8_t arrayIndex = buttonNumber/8;
    uint8_t buttonIndex = buttonNumber - 8*arrayIndex;

    bitWrite(longPressSent[arrayIndex], buttonIndex, state);

}

uint8_t OpenDeck::getButtonType(uint8_t buttonNumber)  {

    uint8_t arrayIndex = buttonNumber/8;
    uint8_t buttonIndex = buttonNumber - 8*arrayIndex;

    if (bitRead(buttonType[arrayIndex], buttonIndex))
        return SYS_EX_BUTTON_TYPE_LATCHING;

    return SYS_EX_BUTTON_TYPE_MOMENTARY;

}

bool OpenDeck::getButtonPPenabled(uint8_t buttonNumber)   {

    uint8_t arrayIndex = buttonNumber/8;
    uint8_t buttonIndex = buttonNumber - 8*arrayIndex;

    return bitRead(buttonPPenabled[arrayIndex], buttonIndex);

}

uint8_t OpenDeck::getButtonNote(uint8_t buttonNumber)   {

    return buttonNote[buttonNumber];

}

bool OpenDeck::getButtonPressed(uint8_t buttonNumber)   {

    uint8_t arrayIndex = buttonNumber/8;
    uint8_t buttonIndex = buttonNumber - 8*arrayIndex;

    return bitRead(buttonPressed[arrayIndex], buttonIndex);

}

bool OpenDeck::getButtonLongPressed(uint8_t buttonNumber)   {

    uint8_t arrayIndex = buttonNumber/8;
    uint8_t buttonIndex = buttonNumber - 8*arrayIndex;

    return bitRead(longPressSent[arrayIndex], buttonIndex);

}

void OpenDeck::procesButtonReading(uint8_t buttonNumber, uint8_t buttonState)  {

    switch (getButtonType(buttonNumber))   {

        case SYS_EX_BUTTON_TYPE_LATCHING:
        processLatchingButton(buttonNumber, buttonState);
        return;
        break;

        case SYS_EX_BUTTON_TYPE_MOMENTARY:
        processMomentaryButton(buttonNumber, buttonState);
        return;
        break;

        default:
        return;
        break;

    }

}

void OpenDeck::processMomentaryButton(uint8_t buttonNumber, bool buttonState)   {

    if (buttonState)    {

        //send note on only once
        if (!getButtonPressed(buttonNumber))    {

            setButtonPressed(buttonNumber, true);

            if (getButtonPPenabled(buttonNumber))    {
                
                sendButtonPPDataCallback(_buttonPPchannel, buttonNote[buttonNumber]);
                return;

            }

            sendButtonNoteDataCallback(buttonNote[buttonNumber], true, _buttonNoteChannel);

        }

    }   else {  //button is released

            //if long-press is enabled
            if (bitRead(buttonFeatures, SYS_EX_FEATURES_BUTTONS_LONG_PRESS)) {

                //if long-press is already sent
                if (getButtonLongPressed(buttonNumber)) {

                    //send both regular and long press note off
                    sendButtonNoteDataCallback(buttonNote[buttonNumber], false, _buttonNoteChannel);
                    sendButtonNoteDataCallback(buttonNote[buttonNumber], false, _longPressButtonNoteChannel);

                }   else if ((getButtonPressed(buttonNumber)) && !getButtonPPenabled(buttonNumber))
                        //send only regular off note
                        sendButtonNoteDataCallback(buttonNote[buttonNumber], false, _buttonNoteChannel);

                        //reset long-press parameters
                        resetLongPress(buttonNumber);

                        //reset regular press
                        setButtonPressed(buttonNumber, false);

            }   else if (getButtonPressed(buttonNumber))    {

                        if (!getButtonPPenabled(buttonNumber))
                            sendButtonNoteDataCallback(buttonNote[buttonNumber], false, _buttonNoteChannel);

                        setButtonPressed(buttonNumber, false);

                }

        }   handleLongPress(buttonNumber, buttonState);

}

void OpenDeck::processLatchingButton(uint8_t buttonNumber, bool buttonState)    {

        if (buttonState != getPreviousButtonState(buttonNumber)) {

            if (buttonState) {  

                //button is pressed
                //if a button has been already pressed
                if (getButtonPressed(buttonNumber)) {

                    //if longPress is enabled and longPressNote has already been sent
                    if (bitRead(buttonFeatures, SYS_EX_FEATURES_BUTTONS_LONG_PRESS) && getButtonLongPressed(buttonNumber)) {

                        //send both regular and long press note off
                        sendButtonNoteDataCallback(buttonNote[buttonNumber], false, _buttonNoteChannel);
                        sendButtonNoteDataCallback(buttonNote[buttonNumber], false, _longPressButtonNoteChannel);

                        resetLongPress(buttonNumber);

                    } else sendButtonNoteDataCallback(buttonNote[buttonNumber], false, _buttonNoteChannel);

                        //reset pressed state
                        setButtonPressed(buttonNumber, false);

                } else {

                        //send note on
                        sendButtonNoteDataCallback(buttonNote[buttonNumber], true, _buttonNoteChannel);

                        //toggle buttonPressed flag to true
                        setButtonPressed(buttonNumber, true);

                }

            }   else {

                    //reset long-press parameters if button is released and long-press hasn't been sent
                    if (!getButtonLongPressed(buttonNumber)) resetLongPress(buttonNumber);

                }

        }   handleLongPress(buttonNumber, buttonState);

}

void OpenDeck::setHandleButtonNoteSend(void (*fptr)(uint8_t buttonNote, bool buttonState, uint8_t channel))   {

    sendButtonNoteDataCallback = fptr;

}

void OpenDeck::setHandleButtonPPSend(void (*fptr)(uint8_t channel, uint8_t program))   {

    sendButtonPPDataCallback = fptr;

}

void OpenDeck::readButtons(uint8_t currentColumn)    {

    if ((_board != 0) && (bitRead(hardwareEnabled, SYS_EX_HW_CONFIG_BUTTONS)))    {

        uint8_t columnState = 0;

        readButtonColumn(columnState);

        if (columnStable(columnState, currentColumn))    {

            for (int i=0; i<(_numberOfButtonRows); i++)   {

                //extract current bit from çolumnState variable
                //invert extracted bit because of pull-up resistors
                uint8_t buttonState = !((columnState >> i) & 0x01);
                //get current button number based on row and column
                uint8_t buttonNumber = currentColumn+i*_numberOfColumns;
                //get encoder pair number based on buttonNumber and current row
                //uint8_t encoderPair = getEncoderPairNumber(i, buttonNumber);

                //if (!encoderPairEnabled[encoderPair])
                    procesButtonReading(buttonNumber, buttonState);

                //else {
//
                    //processEncoderPair(encoderPair, columnState, i);
                    ////skip next row since it's also part of current encoder
                    //i++;
//
                //}

                updateButtonState(buttonNumber, buttonState);

            }

        }

         lastColumnState[currentColumn] = columnState;

    }

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

void OpenDeck::resetLongPress(uint8_t buttonNumber) {

    setButtonLongPressed(buttonNumber, false);
    longPressCounter[buttonNumber] = 0;

}

void OpenDeck::handleLongPress(uint8_t buttonNumber, bool buttonState) {

    //update longPressCounter if:
    //a) long-press is enabled
    //b) button is pressed
    //c) long-press isn't already sent

    if (getButtonPPenabled(buttonNumber)) return; //disable long-press feature when button is configured to send PP

    if (buttonState && bitRead(buttonFeatures, SYS_EX_FEATURES_BUTTONS_LONG_PRESS) && getButtonPressed(buttonNumber) && !getButtonLongPressed(buttonNumber)) {

        longPressCounter[buttonNumber]++;

        if (longPressCounter[buttonNumber] == longPressColumnPass) {

            sendButtonNoteDataCallback(buttonNote[buttonNumber], true, _longPressButtonNoteChannel);
            setButtonLongPressed(buttonNumber, true);

        }

    }

}

bool OpenDeck::columnStable(uint8_t columnState, uint8_t activeColumn)   {

    //column reading is declared stable if there are numberOfColumnPasses same readings
    if (columnState == lastColumnState[activeColumn])   {

        if (columnPassCounter[activeColumn] < numberOfColumnPasses) columnPassCounter[activeColumn]++;

    }   else columnPassCounter[activeColumn] = 0;

    //iterate over rows if column readings are stable
    return (columnPassCounter[activeColumn] == numberOfColumnPasses);

}