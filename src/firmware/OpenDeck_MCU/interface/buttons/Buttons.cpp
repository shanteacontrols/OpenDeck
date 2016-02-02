#include "Buttons.h"
#include "..\eeprom\Configuration.h"
#include "..\interface\midi\MIDI.h"
#include "..\sysex/SysEx.h"
#include "..\BitManipulation.h"
#include "..\interface\settings\ButtonSettings.h"

const uint8_t buttonDebounceCompare = 0b10000000;

Buttons::Buttons()  {

    //def const

}

void Buttons::init()    {

    const subtype buttonTypeSubtype                   = { MAX_NUMBER_OF_BUTTONS, 0, BUTTON_TYPES-1 };
    const subtype buttonProgramChangeEnabledSubtype   = { MAX_NUMBER_OF_BUTTONS, 0, 1 };
    const subtype buttonMIDIidSubtype                 = { MAX_NUMBER_OF_BUTTONS, 0, 127 };

    const subtype *buttonSubtypeArray[] = {

        &buttonTypeSubtype,
        &buttonProgramChangeEnabledSubtype,
        &buttonMIDIidSubtype

    };

    //define message for sysex configuration
    sysEx.addMessageType(CONF_BUTTON_BLOCK, BUTTON_SUBTYPES);

    for (int i=0; i<BUTTON_SUBTYPES; i++)   {

        //define subtype messages
        sysEx.addMessageSubType(CONF_BUTTON_BLOCK, i, buttonSubtypeArray[i]->parameters, buttonSubtypeArray[i]->lowValue, buttonSubtypeArray[i]->highValue);

    }

}

void Buttons::setButtonPressed(uint8_t buttonID, bool state)   {

    uint8_t arrayIndex = buttonID/8;
    uint8_t buttonIndex = buttonID - 8*arrayIndex;

    bitWrite(buttonPressed[arrayIndex], buttonIndex, state);

}

bool Buttons::getButtonPressed(uint8_t buttonID)   {

    uint8_t arrayIndex = buttonID/8;
    uint8_t buttonIndex = buttonID - 8*arrayIndex;

    return bitRead(buttonPressed[arrayIndex], buttonIndex);

}

void Buttons::processProgramChange(uint8_t buttonID, bool buttonState)   {

    if (buttonState)    {

        if (!getButtonPressed(buttonID))    {

            setButtonPressed(buttonID, true);
            midi.sendProgramChange(getMIDIid(buttonID));
            if (sysEx.configurationEnabled()) sysEx.sendComponentID(CONF_BUTTON_BLOCK, buttonID);

        }

    }   else {

        if (getButtonPressed(buttonID)) {

            setButtonPressed(buttonID, false);

        }

    }

}

void Buttons::processMomentaryButton(uint8_t buttonID, bool buttonState)   {

    if (buttonState)    {

        //send note on only once
        if (!getButtonPressed(buttonID))    {

            setButtonPressed(buttonID, true);

            midi.sendMIDInote(getMIDIid(buttonID), true, velocityOn);
            if (sysEx.configurationEnabled()) sysEx.sendComponentID(CONF_BUTTON_BLOCK, buttonID);

        }

    }   else {  //button is released

            if (getButtonPressed(buttonID))    {

                midi.sendMIDInote(getMIDIid(buttonID), false, velocityOff);
                if (sysEx.configurationEnabled()) sysEx.sendComponentID(CONF_BUTTON_BLOCK, buttonID);

                setButtonPressed(buttonID, false);

            }

        }

}

void Buttons::processLatchingButton(uint8_t buttonID, bool buttonState)    {

    if (buttonState != getPreviousButtonState(buttonID)) {

        if (buttonState) {

            //button is pressed
            //if a button has been already pressed
            if (getButtonPressed(buttonID)) {

                midi.sendMIDInote(getMIDIid(buttonID), false, velocityOff);
                if (sysEx.configurationEnabled()) sysEx.sendComponentID(CONF_BUTTON_BLOCK, buttonID);

                //reset pressed state
                setButtonPressed(buttonID, false);

            } else {

                //send note on
                midi.sendMIDInote(getMIDIid(buttonID), true, velocityOn);
                if (sysEx.configurationEnabled()) sysEx.sendComponentID(CONF_BUTTON_BLOCK, buttonID);

                //toggle buttonPressed flag to true
                setButtonPressed(buttonID, true);

            }

        }

    }

}

void Buttons::update()    {

    if (!board.buttonDataAvailable()) return;

    for (int i=0; i<MAX_NUMBER_OF_BUTTONS; i++) {

        uint8_t buttonState = board.getButtonState(i);

        if (buttonDebounced(i, buttonState))  {

            if (getButtonPCenabled(i))  {

                //ignore momentary/latching modes if button sends program change
                //when in program change, button has latching mode since momentary mode makes no sense
                processProgramChange(i, buttonState);

            }   else {

                switch (getButtonType(i))   {

                    case buttonLatching:
                    processLatchingButton(i, buttonState);
                    break;

                    case buttonMomentary:
                    processMomentaryButton(i, buttonState);
                    break;

                    default:
                    break;

                }

            }

            updateButtonState(i, buttonState);

        }

    }

}

void Buttons::updateButtonState(uint8_t buttonID, uint8_t buttonState) {

    uint8_t arrayIndex = buttonID/8;
    uint8_t buttonIndex = buttonID - 8*arrayIndex;

    //update state if it's different than last one
    if (bitRead(previousButtonState[arrayIndex], buttonIndex) != buttonState)
        bitWrite(previousButtonState[arrayIndex], buttonIndex, buttonState);

}

bool Buttons::getPreviousButtonState(uint8_t buttonID) {

    uint8_t arrayIndex = buttonID/8;
    uint8_t buttonIndex = buttonID - 8*arrayIndex;

    return bitRead(previousButtonState[arrayIndex], buttonIndex);

}

bool Buttons::buttonDebounced(uint8_t buttonID, bool buttonState)   {

    //shift new button reading into previousButtonState
    buttonDebounceCounter[buttonID] = (buttonDebounceCounter[buttonID] << 1) | buttonState | buttonDebounceCompare;

    //if button is debounced, return true
    return ((buttonDebounceCounter[buttonID] == buttonDebounceCompare) || (buttonDebounceCounter[buttonID] == 0xFF));

}

buttonType Buttons::getButtonType(uint8_t buttonID)  {

    return (buttonType)configuration.readParameter(CONF_BUTTON_BLOCK, buttonTypeSection, buttonID);

}

bool Buttons::getButtonPCenabled(uint8_t buttonID)   {

    return configuration.readParameter(CONF_BUTTON_BLOCK, buttonProgramChangeEnabledSection, buttonID);

}

uint8_t Buttons::getMIDIid(uint8_t buttonID)   {

    return configuration.readParameter(CONF_BUTTON_BLOCK, buttonMIDIidSection, buttonID);

}

uint8_t Buttons::getParameter(uint8_t messageType, uint8_t parameterID) {

    switch(messageType) {

        case buttonTypeConf:
        return getButtonType(parameterID);
        break;

        case buttonProgramChangeEnabledConf:
        return getButtonPCenabled(parameterID);
        break;

        case buttonMIDIidConf:
        return getMIDIid(parameterID);
        break;

    }   return 0;
}

bool Buttons::setButtonType(uint8_t buttonID, uint8_t type)  {

    return configuration.writeParameter(CONF_BUTTON_BLOCK, buttonTypeSection, buttonID, type);

}

bool Buttons::setButtonPCenabled(uint8_t buttonID, uint8_t state)  {

    return configuration.writeParameter(CONF_BUTTON_BLOCK, buttonProgramChangeEnabledSection, buttonID, state);

}

bool Buttons::setMIDIid(uint8_t buttonID, uint8_t midiID)    {

    return configuration.writeParameter(CONF_BUTTON_BLOCK, buttonMIDIidSection, buttonID, midiID);

}

bool Buttons::setParameter(uint8_t messageType, uint8_t parameter, uint8_t newParameter)    {

    switch(messageType) {

        case buttonTypeConf:
        return setButtonType(parameter, newParameter);
        break;

        case buttonProgramChangeEnabledConf:
        return setButtonPCenabled(parameter, newParameter);
        break;

        case buttonMIDIidConf:
        return setMIDIid(parameter, newParameter);
        break;

    }   return false;

}

Buttons buttons;