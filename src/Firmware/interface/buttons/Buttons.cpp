#include "Buttons.h"
#include "../../eeprom/Configuration.h"
#include "../../interface/midi/MIDI.h"
#include "../../sysex/SysEx.h"
#include "../../BitManipulation.h"

const uint8_t buttonDebounceCompare = 0b10000000;

Buttons::Buttons()  {

    //def const

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

void Buttons::processMomentaryButton(uint8_t buttonID, bool buttonState, bool sendMIDI)   {

    if (buttonState)    {

        //send note on only once
        if (!getButtonPressed(buttonID))    {

            setButtonPressed(buttonID, true);
            midi.sendMIDInote(configuration.readParameter(CONF_BLOCK_BUTTON, buttonMIDIidSection, buttonID), true, velocityOn);
            //if (sysEx.configurationEnabled())
                //sysEx.sendComponentID(CONF_BLOCK_BUTTON, buttonID);

        }

    }   else {  //button is released

            if (getButtonPressed(buttonID))    {

                midi.sendMIDInote(configuration.readParameter(CONF_BLOCK_BUTTON, buttonMIDIidSection, buttonID), false, velocityOff);
                //if (sysEx.configurationEnabled())
                    //sysEx.sendComponentID(CONF_BLOCK_BUTTON, buttonID);

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

                midi.sendMIDInote(configuration.readParameter(CONF_BLOCK_BUTTON, buttonMIDIidSection, buttonID), false, velocityOff);
                //if (sysEx.configurationEnabled())
                    //sysEx.sendComponentID(CONF_BLOCK_BUTTON, buttonID);

                //reset pressed state
                setButtonPressed(buttonID, false);

            } else {

                //send note on
                midi.sendMIDInote(configuration.readParameter(CONF_BLOCK_BUTTON, buttonMIDIidSection, buttonID), true, velocityOn);
                //if (sysEx.configurationEnabled())
                    //sysEx.sendComponentID(CONF_BLOCK_BUTTON, buttonID);

                //toggle buttonPressed flag to true
                setButtonPressed(buttonID, true);

            }

        }

    }

}

void Buttons::processButton(uint8_t buttonID, bool state, bool debounce)   {

    bool debounced = debounce ? buttonDebounced(buttonID, state) : true;

    if (debounced)  {

        buttonType_t type = (buttonType_t)configuration.readParameter(CONF_BLOCK_BUTTON, buttonTypeSection, buttonID);

        if (configuration.readParameter(CONF_BLOCK_BUTTON, buttonProgramChangeEnabledSection, buttonID))  {

            //ignore momentary/latching modes if button sends program change
            //when released, don't send anything
            processMomentaryButton(buttonID, state, state);

        }   else {

            switch (type)   {

                case buttonMomentary:
                processMomentaryButton(buttonID, state);
                break;

                case buttonLatching:
                processLatchingButton(buttonID, state);
                break;

                default:
                break;

            }

        }

        updateButtonState(buttonID, state);

    }

}

void Buttons::update()    {

    if (!core.buttonDataAvailable()) return;

    for (int i=0; i<MAX_NUMBER_OF_BUTTONS; i++) {

        bool buttonState = core.getButtonState(i);
        processButton(i, buttonState);

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

Buttons buttons;
