/*

OpenDECK library v1.0
File: Buttons.cpp
Last revision date: 2014-09-17
Author: Igor Petrovic

*/

#include "OpenDeck.h"
#include "Ownduino.h"


void OpenDeck::setNumberOfColumnPasses() {

    /*

        Algorithm calculates how many times does it need to read whole row
        before it can declare button reading stable.

    */

    uint8_t rowPassTime = getTimedLoopTime()*_numberOfColumns;
    uint8_t mod = 0;

    if ((BUTTON_DEBOUNCE_TIME % rowPassTime) > 0)   mod = 1;

    uint8_t numberOfColumnPasses = ((BUTTON_DEBOUNCE_TIME / rowPassTime) + mod);

    setButtonDebounceCompare(numberOfColumnPasses);

}

void OpenDeck::setButtonDebounceCompare(uint8_t numberOfColumnPasses)   {

    //depending on numberOfColumnPasses, button state gets shifted into
    //different buttonDebounceCompare variable

    switch(numberOfColumnPasses)    {

        case 1:
        buttonDebounceCompare = 0b11111110;
        break;

        case 2:
        buttonDebounceCompare = 0b11111100;
        break;

        case 3:
        buttonDebounceCompare = 0b11111000;
        break;

        case 4:
        buttonDebounceCompare = 0b11110000;
        break;

        default:
        break;

    }

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

    if (!bitRead(buttonType[arrayIndex], buttonIndex))
        return SYS_EX_BUTTON_TYPE_MOMENTARY;

    return SYS_EX_BUTTON_TYPE_LATCHING;

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

bool OpenDeck::checkButton(uint8_t buttonNumber, uint8_t buttonState)  {

    //shift new button reading into buttonState
    buttonState = (previousButtonState[buttonNumber] << 1) | buttonState | buttonDebounceCompare;

    //if buttonState is changed or if button is debounced, return true
    if ((buttonState != previousButtonState[buttonNumber]) || (buttonState == 0xFF))
        return true;

    return false;

}

void OpenDeck::procesButtonReading(uint8_t buttonNumber, uint8_t buttonState)  {

    if (buttonState)    buttonState = 0xFF;
    else                buttonState = buttonDebounceCompare;

    if (buttonState != previousButtonState[buttonNumber])    {

        if (buttonState == 0xFF)    {

            //button is pressed
            //button type is latching
            if (getButtonType(buttonNumber) == SYS_EX_BUTTON_TYPE_LATCHING)    {

                //if a button has been already pressed
                if (getButtonPressed(buttonNumber)) {

                    //if longPress is enabled and longPressNote has already been sent
                    if (bitRead(softwareFeatures, EEPROM_SW_F_LONG_PRESS) && getButtonLongPressed(buttonNumber))   {

                        //send both regular and long press note off
                        sendButtonDataCallback(buttonNote[buttonNumber], false, _buttonNoteChannel);
                        sendButtonDataCallback(buttonNote[buttonNumber], false, _longPressButtonNoteChannel);

                    }   else    sendButtonDataCallback(buttonNote[buttonNumber], false, _buttonNoteChannel);

                    //reset pressed state
                    setButtonPressed(buttonNumber, false);

                    }   else    {

                    //send note on on press
                    sendButtonDataCallback(buttonNote[buttonNumber], true, _buttonNoteChannel);

                    //toggle buttonPressed flag to true
                    setButtonPressed(buttonNumber, true);

                }

            }   else    sendButtonDataCallback(buttonNote[buttonNumber], true, _buttonNoteChannel);

            //start long press timer
            if (bitRead(softwareFeatures, EEPROM_SW_F_LONG_PRESS)) longPressState[buttonNumber] = millis();

            }   else    if ((buttonState == buttonDebounceCompare) && (!getButtonType(buttonNumber)))   {

            //button is released
            //check button on release only if it's momentary

            if (bitRead(softwareFeatures, EEPROM_SW_F_LONG_PRESS)) {

                if (getButtonLongPressed(buttonNumber)) {

                    //send both regular and long press note off
                    sendButtonDataCallback(buttonNote[buttonNumber], false, _buttonNoteChannel);
                    sendButtonDataCallback(buttonNote[buttonNumber], false, _longPressButtonNoteChannel);

                }   else    sendButtonDataCallback(buttonNote[buttonNumber], false, _buttonNoteChannel);

                longPressState[buttonNumber] = 0;
                setButtonLongPressed(buttonNumber, false);

            }   else    sendButtonDataCallback(buttonNote[buttonNumber], false, _buttonNoteChannel);

        }

        //update previous reading with current
        previousButtonState[buttonNumber] = buttonState;

    }

    if (bitRead(softwareFeatures, EEPROM_SW_F_LONG_PRESS)) {

        //send long press note if button has been pressed for defined time and note hasn't already been sent
        if ((millis() - longPressState[buttonNumber] >= _longPressTime) &&
            (!getButtonLongPressed(buttonNumber)) && (buttonState == 0xFF))  {

                sendButtonDataCallback(buttonNote[buttonNumber], true, _longPressButtonNoteChannel);
                setButtonLongPressed(buttonNumber, true);

        }

    }

}

void OpenDeck::setHandleButtonSend(void (*fptr)(uint8_t buttonNote, bool buttonState, uint8_t channel))   {

    sendButtonDataCallback = fptr;

}

void OpenDeck::readButtons()    {

    if ((_board != 0) && (bitRead(hardwareFeatures, EEPROM_HW_F_BUTTONS)))    {

        uint8_t columnState = 0;

        readButtonColumn(columnState);

        //iterate over rows
        for (int i=0; i<(_numberOfButtonRows+freePinsAsBRows); i++)   {

            //extract current bit from çolumnState variable
            //invert extracted bit because of pull-up resistors
            uint8_t buttonState = !((columnState >> i) & 0x01);
            //get current button number based on row and column
            uint8_t buttonNumber = getActiveColumn()+i*_numberOfColumns;

            if (checkButton(buttonNumber, buttonState))
                procesButtonReading(buttonNumber, buttonState);

        }

    }

}
