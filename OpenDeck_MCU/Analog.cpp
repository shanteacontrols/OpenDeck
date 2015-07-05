/*

OpenDECK library v1.3
File: Potentiometers.cpp
Last revision date: 2014-12-25
Author: Igor Petrovic

*/

#include "OpenDeck.h"
#include <avr/eeprom.h>
#include <Ownduino.h>
#include <util/delay.h>

#define NUMBER_OF_SAMPLES 3

//potentiometer must exceed this value before sending new value
#define MIDI_CC_STEP_REGULAR        8
#define MIDI_CC_STEP_DEBOUNCE       16
#define ANALOG_DEBOUNCE_TIMEOUT     80


void OpenDeck::readAnalog()   {

    #ifdef BOARD

    if (boardObject.analogInDataAvailable())    {

        //check one multiplexer at once, each has 8 inputs
        for (int i=0; i<8; i++)    {

            int16_t analogData = boardObject.getAnalogValue(i);
            uint8_t analogID = boardObject.getAnalogID(i);

            if (getAnalogEnabled(analogID))   {

                if (checkAnalogReading(analogData, analogID)) {

                    analogData = getMedianValue(analogID);
                    processAnalogReading(analogData, analogID);

                }

            }

        }   boardObject.setAnalogProcessingFinished(true);

    }

    #endif

}

void OpenDeck::readAnalogInitial()   {

   for (int i=0; i<NUMBER_OF_MUX; i++)  {

       boardObject.setMux(i);

       for (int j=0; j<8; j++)  {

           boardObject.setMuxInput(j);
           lastAnalogueValue[j+i*8] = getADCvalue();

       }

   }

}

bool OpenDeck::checkAnalogReading(int16_t tempValue, uint8_t potNumber) {

    //calculate difference between current and previous reading
    int16_t analogueDiff = tempValue - lastAnalogueValue[potNumber];

    //get absolute difference
    if (analogueDiff < 0)   analogueDiff *= -1;

    uint8_t stepsNeeded;

    if (millisOwnduino() - analogTimer[potNumber] > ANALOG_DEBOUNCE_TIMEOUT) stepsNeeded = MIDI_CC_STEP_DEBOUNCE;
    else stepsNeeded = MIDI_CC_STEP_REGULAR;

    if (analogueDiff >= stepsNeeded)   {

        analogSample[potNumber][analogDebounceCounter[potNumber]] = tempValue;
        analogDebounceCounter[potNumber]++;

        if (analogDebounceCounter[potNumber] == NUMBER_OF_SAMPLES) {

            analogDebounceCounter[potNumber] = 0;
            analogTimer[potNumber] = millisOwnduino();
            return true;

        }

    }

    return false;

}

void OpenDeck::processAnalogReading(int16_t tempValue, uint8_t potNumber)  {

    uint8_t ccValue = tempValue >> 3;

    //invert CC data if potInverted is true
    if (getAnalogInvertState(potNumber))   ccValue = 127 - ccValue;

    //only send data if function isn't called in setup
    if (sendControlChangeCallback != NULL)  {

        //only use map when cc limits are different from defaults
        if ((analogLowerLimit[potNumber] != 0) || (analogUpperLimit[potNumber] != 127))
        sendControlChangeCallback(analogNumber[potNumber], map_uint8(ccValue, 0, 127, analogLowerLimit[potNumber], analogUpperLimit[potNumber]), _analogCCchannel);

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

int16_t OpenDeck::getMedianValue(uint8_t analogID)  {

    int16_t medianValue = 0;

    if ((analogSample[analogID][0] <= analogSample[analogID][1]) && (analogSample[analogID][0] <= analogSample[analogID][2]))
    {
        medianValue = (analogSample[analogID][1] <= analogSample[analogID][2]) ? analogSample[analogID][1] : analogSample[analogID][2];
    }
    else if ((analogSample[analogID][1] <= analogSample[analogID][0]) && (analogSample[analogID][1] <= analogSample[analogID][2]))
    {
        medianValue = (analogSample[analogID][0] <= analogSample[analogID][2]) ? analogSample[analogID][0] : analogSample[analogID][2];
    }
    else
    {
        medianValue = (analogSample[analogID][0] <= analogSample[analogID][1]) ? analogSample[analogID][0] : analogSample[analogID][1];
    }

    return medianValue;

}