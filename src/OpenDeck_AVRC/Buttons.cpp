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

subtype buttonTypeSubtype                   = { MAX_NUMBER_OF_BUTTONS, 0, BUTTON_TYPES-1, EEPROM_BUTTONS_TYPE_START, BIT_PARAMETER };
subtype buttonProgramChangeEnabledSubtype   = { MAX_NUMBER_OF_BUTTONS, 0, 1, EEPROM_BUTTONS_PC_ENABLED_START, BIT_PARAMETER };
subtype buttonMIDIidSubtype                 = { MAX_NUMBER_OF_BUTTONS, 0, 127, EEPROM_BUTTONS_NOTE_START, BYTE_PARAMETER };

const subtype *buttonSubtypeArray[] = {

    &buttonTypeSubtype,
    &buttonProgramChangeEnabledSubtype,
    &buttonMIDIidSubtype

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
        sysEx.addMessageSubType(SYS_EX_MT_BUTTONS, i, buttonSubtypeArray[i]->parameters, buttonSubtypeArray[i]->lowValue, buttonSubtypeArray[i]->highValue);

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

    if (!board.buttonDataAvailable()) return;

    for (int i=0; i<MAX_NUMBER_OF_BUTTONS; i++) {

        uint8_t buttonState = board.getButtonState(i);

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

    return eepromSettings.writeParameter(buttonSubtypeArray[buttonTypeConf]->eepromAddress, buttonID, type, buttonSubtypeArray[buttonTypeConf]->parameterType);

}

bool Buttons::setButtonPCenabled(uint8_t buttonID, uint8_t state)  {

    return eepromSettings.writeParameter(buttonSubtypeArray[buttonProgramChangeEnabledConf]->eepromAddress, buttonID, state, buttonSubtypeArray[buttonProgramChangeEnabledConf]->parameterType);

}

bool Buttons::setMIDIid(uint8_t buttonID, uint8_t midiID)    {

    return eepromSettings.writeParameter(buttonSubtypeArray[buttonMIDIidConf]->eepromAddress, buttonID, midiID, buttonSubtypeArray[buttonMIDIidConf]->parameterType);

}

bool Buttons::setParameter(uint8_t messageType, uint8_t parameter, uint8_t newParameter)    {

    return eepromSettings.writeParameter(buttonSubtypeArray[messageType]->eepromAddress, parameter, newParameter, buttonSubtypeArray[messageType]->parameterType);

}

buttonType Buttons::getButtonType(uint8_t buttonID)  {

    return (buttonType)eepromSettings.readParameter(buttonSubtypeArray[buttonTypeConf]->eepromAddress, buttonID, buttonSubtypeArray[buttonTypeConf]->parameterType);

}

bool Buttons::getButtonPCenabled(uint8_t buttonID)   {

    return eepromSettings.readParameter(buttonSubtypeArray[buttonProgramChangeEnabledConf]->eepromAddress, buttonID, buttonSubtypeArray[buttonProgramChangeEnabledConf]->parameterType);

}

uint8_t Buttons::getMIDIid(uint8_t buttonID)   {

    return eepromSettings.readParameter(buttonSubtypeArray[buttonMIDIidConf]->eepromAddress, buttonID, buttonSubtypeArray[buttonMIDIidConf]->parameterType);

}

uint8_t Buttons::getParameter(uint8_t messageType, uint8_t parameterID) {

    return eepromSettings.readParameter(buttonSubtypeArray[messageType]->eepromAddress, parameterID, buttonSubtypeArray[messageType]->parameterType);

}

Buttons buttons;