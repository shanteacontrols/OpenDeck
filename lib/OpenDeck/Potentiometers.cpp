/*

OpenDECK library v1.2
File: Potentiometers.cpp
Last revision date: 2014-11-18
Author: Igor Petrovic

*/

#include "OpenDeck.h"
#include <avr/eeprom.h>
#include "Ownduino.h"

//potentiometer must exceed this value before sending new value
#define MIDI_CC_STEP                8

//potentiometer must exceed this value if it hasn't been moved for more than POTENTIOMETER_MOVE_TIMEOUT
#define MIDI_CC_STEP_TIMEOUT        10

//time in ms after which new value from pot must exceed MIDI_CC_STEP_TIMEOUT
#define POTENTIOMETER_MOVE_TIMEOUT  200

void OpenDeck::setHandlePotCC(void (*fptr)(uint8_t potNumber, uint8_t ccValue, uint8_t channel))    {

    sendPotCCDataCallback = fptr;

}

void OpenDeck::setHandlePotNoteOn(void (*fptr)(uint8_t note, uint8_t channel))   {

    sendPotNoteOnDataCallback = fptr;

}

void OpenDeck::setHandlePotNoteOff(void (*fptr)(uint8_t note, uint8_t channel))  {

    sendPotNoteOffDataCallback = fptr;

}

void OpenDeck::readPots()   {

    if ((_board != 0) && (bitRead(hardwareFeatures, EEPROM_HW_F_POTS)))    {

        static int8_t previousMuxNumber = -1;
        uint8_t currentMuxNumber = getActiveMux();

        if (previousMuxNumber != currentMuxNumber)   {

            for (int i=0; i<8; i++) {

                setMuxInput(i);
                readPotsMux(i, currentMuxNumber);

            }

            previousMuxNumber = currentMuxNumber;

        }

    }

}

void OpenDeck::readPotsMux(uint8_t muxInput, uint8_t muxNumber)  {

        //calculate pot number
        uint8_t potNumber = muxNumber*8+muxInput;

        //don't read/process data from pot if it's disabled
        if (getPotEnabled(potNumber))   {

            //read analogue value from mux
            int16_t tempValue = getADCvalue();

            //if new reading is stable, send new MIDI message
            if (checkPotReading(tempValue, potNumber))
                processPotReading(tempValue, potNumber);

        }

}

uint8_t OpenDeck::adcConnected(uint8_t muxNumber) {

    return analogueEnabledArray[muxNumber];

}

bool OpenDeck::checkPotReading(int16_t tempValue, uint8_t potNumber) {

    //calculate difference between current and previous reading
    int8_t analogueDiff = tempValue - lastAnalogueValue[potNumber];

    //get absolute difference
    if (analogueDiff < 0)   analogueDiff *= -1;

    uint32_t timeDifference = millis() - potTimer[potNumber];

        /*

            When value from pot hasn't changed for more than POTENTIOMETER_MOVE_TIMEOUT value (time in ms), pot must
            exceed MIDI_CC_STEP_TIMEOUT value. If the value has changed during POTENTIOMETER_MOVE_TIMEOUT, it must
            exceed MIDI_CC_STEP value.

        */

        if (timeDifference < POTENTIOMETER_MOVE_TIMEOUT)    {

            if (analogueDiff >= MIDI_CC_STEP)   return true;

        }   else    {

                if (analogueDiff >= MIDI_CC_STEP_TIMEOUT)   return true;

            }

    return false;

}

void OpenDeck::processPotReading(int16_t tempValue, uint8_t potNumber)  {

    uint8_t ccValue;
    uint8_t potNoteChannel = _longPressButtonNoteChannel+1;

    //invert CC data if potInverted is true
    if (getPotInvertState(potNumber))   ccValue = 127 - (tempValue >> 3);
    else                                ccValue = tempValue >> 3;

    //only send data if function isn't called in setup
    if (sendPotCCDataCallback != NULL)  {

        //only use map when cc limits are different from defaults
        if ((ccLowerLimit[potNumber] != 0) || (ccUpperLimit[potNumber] != 127))
            sendPotCCDataCallback(ccNumber[potNumber], map(ccValue, 0, 127, ccLowerLimit[potNumber], ccUpperLimit[potNumber]), _potCCchannel);

        else    sendPotCCDataCallback(ccNumber[potNumber], ccValue, _potCCchannel);

    }

    if (bitRead(softwareFeatures, EEPROM_SW_F_POT_NOTES))  {

        uint8_t noteCurrent = getPotNoteValue(ccValue, ccNumber[potNumber]);

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
                sendPotNoteOffDataCallback(lastPotNoteValue[ccNumber[potNumber]], _potNoteChannel);

            //send note on
            if ((sendPotNoteOnDataCallback != NULL) && (getPotEnabled(potNumber)))
                sendPotNoteOnDataCallback(noteCurrent, _potNoteChannel);

            //update last value with current
            lastPotNoteValue[potNumber] = noteCurrent;;

        }

    }

    //update values
    lastAnalogueValue[potNumber] = tempValue;
    potTimer[potNumber] = millis();

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

bool OpenDeck::getPotInvertState(uint8_t potNumber) {

    uint8_t arrayIndex = potNumber/8;
    uint8_t potIndex = potNumber - 8*arrayIndex;

    return bitRead(potInverted[arrayIndex], potIndex);

}

uint8_t OpenDeck::getCCnumber(uint8_t potNumber)    {

    return ccNumber[potNumber];

}