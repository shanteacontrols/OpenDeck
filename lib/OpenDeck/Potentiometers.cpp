/*

OpenDECK library v1.3
File: Potentiometers.cpp
Last revision date: 2014-12-25
Author: Igor Petrovic

*/

#include "OpenDeck.h"
#include <avr/eeprom.h>
#include "Ownduino.h"
#include <util/delay.h>

#define NUMBER_OF_SAMPLES 3

//potentiometer must exceed this value before sending new value
#define MIDI_CC_STEP                2

void OpenDeck::readAnalog()   {

    #ifdef BOARD

        if (boardObject.analogInDataAvailable()) {

            for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)    {

                int16_t analogData = boardObject.getAnalogInData(i);

                if (getAnalogEnabled(i))   {

                    if (checkAnalogReading(analogData, i)) {

                        processAnalogReading(analogData, i);

                    }

                }

            }

            boardObject.startAnalogConversion();

        }

    #endif

}

void OpenDeck::readAnalogInitial()   {

    //wait until analog buffer is full, and then update all analog value whether
    //they're stable or not to avoid sending all analog data on power on
    while (!boardObject.analogInDataAvailable())    {};
        for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
            lastAnalogueValue[i] = boardObject.getAnalogInData(i);

     //restart analog readouts
     boardObject.startAnalogConversion();

}

bool OpenDeck::checkAnalogReading(int16_t tempValue, uint8_t potNumber) {

    //calculate difference between current and previous reading
    int8_t analogueDiff = tempValue - lastAnalogueValue[potNumber];

    //get absolute difference
    if (analogueDiff < 0)   analogueDiff *= -1;

    if (analogueDiff >= MIDI_CC_STEP)   {

        analogDebounceCounter[potNumber]++;
        if (analogDebounceCounter[potNumber] == NUMBER_OF_SAMPLES) {

            analogDebounceCounter[potNumber] = 0;
            return true;

        }

    }

    return false;

}

void OpenDeck::processAnalogReading(int16_t tempValue, uint8_t potNumber)  {

    uint8_t ccValue = tempValue >> 1;

    //invert CC data if potInverted is true
    if (getAnalogInvertState(potNumber))   ccValue = 127 - ccValue;

    //only send data if function isn't called in setup
    if (sendControlChangeCallback != NULL)  {

        //only use map when cc limits are different from defaults
        if ((analogLowerLimit[potNumber] != 0) || (analogUpperLimit[potNumber] != 127))
            sendControlChangeCallback(analogNumber[potNumber], map(ccValue, 0, 127, analogLowerLimit[potNumber], analogUpperLimit[potNumber]), _analogCCchannel);

        else    sendControlChangeCallback(analogNumber[potNumber], ccValue, _analogCCchannel);

    }

    //update values
    lastAnalogueValue[potNumber] = tempValue;

}

bool OpenDeck::getAnalogEnabled(uint8_t potNumber) {

    uint8_t arrayIndex = potNumber/8;
    uint8_t potIndex = potNumber - 8*arrayIndex;

    return bitRead(analogEnabled[arrayIndex], potIndex);

}

uint8_t OpenDeck::getAnalogType(uint8_t potNumber) {

    return analogType[potNumber];

}

bool OpenDeck::getAnalogInvertState(uint8_t potNumber) {

    uint8_t arrayIndex = potNumber/8;
    uint8_t potIndex = potNumber - 8*arrayIndex;

    return bitRead(analogInverted[arrayIndex], potIndex);

}

uint8_t OpenDeck::getAnalogNumber(uint8_t potNumber)    {

    return analogNumber[potNumber];

}