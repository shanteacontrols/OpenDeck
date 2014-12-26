/*

OpenDECK library v1.3
File: Potentiometers.cpp
Last revision date: 2014-12-25
Author: Igor Petrovic

*/

#include "OpenDeck.h"
#include <avr/eeprom.h>
#include "Ownduino.h"

//potentiometer must exceed this value before sending new value
#define MIDI_CC_STEP                2

void OpenDeck::setHandlePotCC(void (*fptr)(uint8_t potNumber, uint8_t ccValue, uint8_t channel))    {

    sendPotCCDataCallback = fptr;

}

void OpenDeck::setHandlePotNoteOn(void (*fptr)(uint8_t note, uint8_t channel))   {

    sendPotNoteOnDataCallback = fptr;

}

void OpenDeck::setHandlePotNoteOff(void (*fptr)(uint8_t note, uint8_t channel))  {

    sendPotNoteOffDataCallback = fptr;

}

uint8_t OpenDeck::getPotNumber(uint8_t muxNumber, uint8_t muxInput) {

    return muxNumber*8+muxInput;

}

void OpenDeck::readPots()   {

    if ((_board != 0) && (bitRead(hardwareEnabled, SYS_EX_HW_CONFIG_POTS)))    {

        static int8_t previousMuxNumber = -1;
        int8_t currentMuxNumber = getActiveMux();

        if (previousMuxNumber != currentMuxNumber)   {

            for (int i=0; i<8; i++) {

                setMuxInput(i);
                readPotsMux(i, currentMuxNumber);

            }

            previousMuxNumber = currentMuxNumber;

        }

    }

}

void OpenDeck::readPotsInitial()   {

    if ((_board != 0) && (bitRead(hardwareEnabled, SYS_EX_HW_CONFIG_POTS)))    {

        for (int muxNumber=0; muxNumber<_numberOfMux; muxNumber++)  {

            for (int muxInput=0; muxInput<8; muxInput++) {

                setMuxInput(muxInput);
                //store read values right after reading them
                lastAnalogueValue[getPotNumber(muxNumber, muxInput)] = analogRead(getMuxPin(muxNumber));

            }

        }

    }

}

void OpenDeck::readPotsMux(uint8_t muxInput, uint8_t muxNumber)  {

        //calculate pot number
        uint8_t potNumber = getPotNumber(muxNumber, muxInput);

        //don't read/process data from pot if it's disabled
        if (getPotEnabled(potNumber))   {

            //read analogue value from mux
            int16_t tempValue = getADCvalue();

            //if new reading is stable, send new MIDI message
            if (checkPotReading(tempValue, potNumber))
                processPotReading(tempValue, potNumber);

        }

}

uint8_t OpenDeck::getMuxPin(uint8_t muxNumber) {

    //returns pin on which muxNumber is connected
    return analogueEnabledArray[muxNumber];

}

bool OpenDeck::checkPotReading(int16_t tempValue, uint8_t potNumber) {

    //calculate difference between current and previous reading
    int8_t analogueDiff = tempValue - lastAnalogueValue[potNumber];

    //get absolute difference
    if (analogueDiff < 0)   analogueDiff *= -1;

    if (analogueDiff >= MIDI_CC_STEP)   return true;
    return false;

}

void OpenDeck::processPotReading(int16_t tempValue, uint8_t potNumber)  {

    uint8_t ccValue;
    uint8_t potNoteChannel = _longPressButtonNoteChannel+1;

    //invert CC data if potInverted is true
    if (getPotInvertState(potNumber))   ccValue = 127 - (tempValue >> 1);
    else                                ccValue = tempValue >> 1;

    //only send data if function isn't called in setup
    if (sendPotCCDataCallback != NULL)  {

        //only use map when cc limits are different from defaults
        if ((ccLowerLimit[potNumber] != 0) || (ccUpperLimit[potNumber] != 127))
            sendPotCCDataCallback(ccppNumber[potNumber], map(ccValue, 0, 127, ccLowerLimit[potNumber], ccUpperLimit[potNumber]), _potCCchannel);

        else    sendPotCCDataCallback(ccppNumber[potNumber], ccValue, _potCCchannel);

    }

    if (bitRead(potFeatures, SYS_EX_FEATURES_POTS_NOTES))  {

        uint8_t noteCurrent = getPotNoteValue(ccValue, ccppNumber[potNumber]);

        //maximum number of notes per MIDI channel is 128, with 127 being final
        if (noteCurrent > 127)  {

            //if calculated note is bigger than 127, assign next midi channel
            potNoteChannel += noteCurrent/128;

            //substract 128*number of overflown channels from note
            noteCurrent -= 128*(noteCurrent/128);

        }

        if (checkPotNoteValue(potNumber, noteCurrent))  {

            //always send note off for previous value, except for the first read
            if ((lastPotNoteValue[potNumber] != 128) && (sendPotNoteOffDataCallback != NULL) && (getPotEnabled(potNumber)))
                sendPotNoteOffDataCallback(lastPotNoteValue[ccppNumber[potNumber]], _potNoteChannel);

            //send note on
            if ((sendPotNoteOnDataCallback != NULL) && (getPotEnabled(potNumber)))
                sendPotNoteOnDataCallback(noteCurrent, _potNoteChannel);

            //update last value with current
            lastPotNoteValue[potNumber] = noteCurrent;;

        }

    }

    //update values
    lastAnalogueValue[potNumber] = tempValue;

}

uint8_t OpenDeck::getPotNoteValue(uint8_t analogueMIDIvalue, uint8_t potNumber) {

    /*

    Each potentiometer alongside regular CC messages sends 6 different MIDI notes,
    depending on it's position. In the following table x represents the reading from
    pot and right column the MIDI note number:

    x=0:        0
    0<x<32:     1
    32<=x<64:   2
    64<=x<96:   3
    96<=x<127:  4
    x=127:      5

    */

    //variable to hold current modifier value
    uint8_t modifierValue = 6*potNumber;

    switch (analogueMIDIvalue)  {

    case 0:
    modifierValue += 0;
    break;

    case 127:
    modifierValue += 5;
    break;

    default:
    modifierValue += 1 + (analogueMIDIvalue >> 5);
    break;

    }

    return modifierValue;

}

bool OpenDeck::checkPotNoteValue(uint8_t potNumber, uint8_t noteCurrent)    {

    //make sure that modifier value is sent only once while the pot is in specified range
    if (lastPotNoteValue[potNumber] != noteCurrent) return true;

    return false;

}

bool OpenDeck::getPotEnabled(uint8_t potNumber) {

    uint8_t arrayIndex = potNumber/8;
    uint8_t potIndex = potNumber - 8*arrayIndex;

    return bitRead(potEnabled[arrayIndex], potIndex);

}

bool OpenDeck::getPotPPenabled(uint8_t potNumber) {

    uint8_t arrayIndex = potNumber/8;
    uint8_t potIndex = potNumber - 8*arrayIndex;

    return bitRead(potPPenabled[arrayIndex], potIndex);

}

bool OpenDeck::getPotInvertState(uint8_t potNumber) {

    uint8_t arrayIndex = potNumber/8;
    uint8_t potIndex = potNumber - 8*arrayIndex;

    return bitRead(potInverted[arrayIndex], potIndex);

}

uint8_t OpenDeck::getCCnumber(uint8_t potNumber)    {

    return ccppNumber[potNumber];

}