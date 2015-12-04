#include "Buttons.h"
#include "sysex/ProtocolDefinitions.h"
#include "eeprom/EEPROMsettings.h"
#include "..\midi\MIDI.h"
#include "sysex/SysEx.h"
#include "BitManipulation.h"

typedef enum {

    buttonTypeConf,
    buttonProgramChangeEnabledConf,
    buttonMIDIidConf,
    BUTTONS_SUBTYPES

} sysExMessageSubTypeButtons;

subtype buttonTypeSubtype                   = { MAX_NUMBER_OF_BUTTONS, 0, (BUTTON_TYPES-1) };
subtype buttonProgramChangeEnabledSubtype   = { MAX_NUMBER_OF_BUTTONS, 0, 1 };
subtype buttonMIDIidSubtype                 = { MAX_NUMBER_OF_BUTTONS, 0, 127 };

const uint8_t buttonSubtypeArray[] = {

    buttonTypeConf,
    buttonProgramChangeEnabledConf,
    buttonMIDIidConf

};

const uint8_t buttonParametersArray[] = {

    buttonTypeSubtype.parameters,
    buttonProgramChangeEnabledSubtype.parameters,
    buttonMIDIidSubtype.parameters

};

const uint8_t buttonNewParameterLowArray[] = {

    buttonTypeSubtype.lowValue,
    buttonProgramChangeEnabledSubtype.lowValue,
    buttonMIDIidSubtype.lowValue

};

const uint8_t buttonNewParameterHighArray[] = {

    buttonTypeSubtype.highValue,
    buttonProgramChangeEnabledSubtype.highValue,
    buttonMIDIidSubtype.highValue

};

const uint8_t buttonDebounceCompare = 0b10000000;

Buttons::Buttons()  {

    //def const

}

void Buttons::init()    {

    //define message for sysex configuration
    sysEx.addMessageType(SYS_EX_MT_BUTTONS, BUTTONS_SUBTYPES);

    for (int i=0; i<BUTTONS_SUBTYPES; i++)   {

        //define subtype messages
        sysEx.addMessageSubType(SYS_EX_MT_BUTTONS, buttonSubtypeArray[i], buttonParametersArray[i], buttonNewParameterLowArray[i], buttonNewParameterHighArray[i]);

    }

}

void Buttons::setButtonPressed(uint8_t buttonID, bool state)   {

    uint8_t arrayIndex = buttonID/8;
    uint8_t buttonIndex = buttonID - 8*arrayIndex;

    bitWrite(buttonPressed[arrayIndex], buttonIndex, state);

}

buttonType Buttons::getButtonType(uint8_t buttonID)  {

    uint8_t arrayIndex = buttonID/8;
    uint8_t buttonIndex = buttonID - 8*arrayIndex;
    uint8_t buttonTypeArray = eeprom_read_byte((uint8_t*)EEPROM_BUTTONS_TYPE_START+arrayIndex);

    if (bitRead(buttonTypeArray, buttonIndex))
        return buttonLatching;
    return buttonMomentary;

}

bool Buttons::getButtonPCenabled(uint8_t buttonID)   {

    uint8_t arrayIndex = buttonID/8;
    uint8_t buttonIndex = buttonID - 8*arrayIndex;
    uint8_t buttonPCenabledArray = eeprom_read_byte((uint8_t*)EEPROM_BUTTONS_PC_ENABLED_START+arrayIndex);

    return bitRead(buttonPCenabledArray, buttonIndex);

}

uint8_t Buttons::getMIDIid(uint8_t buttonID)   {

    return eeprom_read_byte((uint8_t*)EEPROM_BUTTONS_NOTE_START+buttonID);

}

bool Buttons::getButtonPressed(uint8_t buttonID)   {

    uint8_t arrayIndex = buttonID/8;
    uint8_t buttonIndex = buttonID - 8*arrayIndex;

    return bitRead(buttonPressed[arrayIndex], buttonIndex);

}

void Buttons::processMomentaryButton(uint8_t buttonID, bool buttonState)   {

    if (buttonState)    {

        //send note on only once
        if (!getButtonPressed(buttonID))    {

            setButtonPressed(buttonID, true);

            if (getButtonPCenabled(buttonID))    {

                midi.sendProgramChange(getMIDIid(buttonID));
                if (sysEx.configurationEnabled()) sysEx.sendID(buttonComponent, buttonID);
                return;

            }

            midi.sendMIDInote(getMIDIid(buttonID), true, velocityOn);
            if (sysEx.configurationEnabled()) sysEx.sendID(buttonComponent, buttonID);

        }

    }   else {  //button is released

            if (getButtonPressed(buttonID))    {

                if (!getButtonPCenabled(buttonID))  {

                    midi.sendMIDInote(getMIDIid(buttonID), false, velocityOff);
                    if (sysEx.configurationEnabled()) sysEx.sendID(buttonComponent, buttonID);

                }

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
                if (sysEx.configurationEnabled()) sysEx.sendID(buttonComponent, buttonID);

                //reset pressed state
                setButtonPressed(buttonID, false);

            } else {

                //send note on
                midi.sendMIDInote(getMIDIid(buttonID), true, velocityOn);
                if (sysEx.configurationEnabled()) sysEx.sendID(buttonComponent, buttonID);

                //toggle buttonPressed flag to true
                setButtonPressed(buttonID, true);

            }

        }

    }

}

void Buttons::update()    {

    if (!boardObject.buttonDataAvailable()) return;

    for (int i=0; i<MAX_NUMBER_OF_BUTTONS; i++) {

        uint8_t buttonState = boardObject.getButtonState(i);

        if (buttonDebounced(i, buttonState))  {

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

bool Buttons::setButtonType(uint8_t buttonID, uint8_t type)  {

    uint8_t arrayIndex = buttonID/8;
    uint8_t buttonIndex = buttonID - 8*arrayIndex;
    uint16_t address = EEPROM_BUTTONS_TYPE_START+arrayIndex;
    uint8_t buttonTypeArray = eeprom_read_byte((uint8_t*)address);

    if (type == RESET_VALUE)    bitWrite(buttonTypeArray, buttonIndex, bitRead(pgm_read_byte(&(defConf[address])), buttonIndex));
    else                        bitWrite(buttonTypeArray, buttonIndex, type);

    eeprom_update_byte((uint8_t*)address, buttonTypeArray);

    return (buttonTypeArray == eeprom_read_byte((uint8_t*)address));

}

bool Buttons::setButtonPCenabled(uint8_t buttonID, uint8_t state)  {

    uint8_t arrayIndex = buttonID/8;
    uint8_t buttonIndex = buttonID - 8*arrayIndex;
    uint16_t address = EEPROM_BUTTONS_TYPE_START+arrayIndex;
    uint8_t buttonPCenabledArray = eeprom_read_byte((uint8_t*)address);

    if (state == RESET_VALUE)   bitWrite(buttonPCenabledArray, buttonIndex, bitRead(pgm_read_byte(&(defConf[address])), buttonIndex));
    else                        bitWrite(buttonPCenabledArray, buttonIndex, state);

    eeprom_update_byte((uint8_t*)address, buttonPCenabledArray);

    return (buttonPCenabledArray == eeprom_read_byte((uint8_t*)address));

}

uint8_t Buttons::getParameter(uint8_t messageType, uint8_t parameterID) {

    switch(messageType) {

        case buttonTypeConf:
        return getButtonType(parameterID);
        break;

        case buttonProgramChangeEnabledConf:
        return (int8_t)getButtonPCenabled(parameterID);
        break;

        case buttonMIDIidConf:
        return getMIDIid(parameterID);
        break;

    }   return INVALID_VALUE;

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
        return setMIDIid(newParameter, newParameter);
        break;

    }   return false;

}

bool Buttons::setMIDIid(uint8_t buttonID, uint8_t midiID)    {

    uint16_t address = EEPROM_BUTTONS_NOTE_START+buttonID;

    if (midiID == RESET_VALUE) midiID = pgm_read_byte(&(defConf[address]));

    eeprom_update_byte((uint8_t*)address, midiID);

    return (midiID == eeprom_read_byte((uint8_t*)address));

}

Buttons buttons;