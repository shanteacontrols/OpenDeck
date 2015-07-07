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
#define MIDI_CC_STEP        8


void OpenDeck::readAnalog()   {

    #ifdef BOARD

    if (boardObject.analogInDataAvailable())    {

        //read all mux inputs first
        int16_t analogData[8];

        for (int i=0; i<8; i++)
            analogData[i] = boardObject.getMuxInputValue(i);

        //check values
        for (int i=0; i<8; i++)    {

            uint8_t analogID = boardObject.getAnalogID(i);

            if (getAnalogEnabled(analogID))   {

                if (checkAnalogReading(analogData[i], analogID)) {

                    analogData[i] = getMedianValue(analogID);
                    processAnalogReading(analogData[i], analogID);

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

bool OpenDeck::checkAnalogReading(int16_t tempValue, uint8_t analogID) {

    //calculate difference between current and previous reading
    int16_t analogueDiff = tempValue - lastAnalogueValue[analogID];

    //get absolute difference
    if (analogueDiff < 0)   analogueDiff *= -1;

    if (analogueDiff >= MIDI_CC_STEP)   {

        analogSample[analogID][analogDebounceCounter[analogID]] = tempValue;
        analogDebounceCounter[analogID]++;

        if (analogDebounceCounter[analogID] == NUMBER_OF_SAMPLES) {

            analogDebounceCounter[analogID] = 0;
            return true;

        }

    }

    return false;

}

void OpenDeck::processAnalogReading(int16_t tempValue, uint8_t analogID)  {

    uint8_t ccValue = tempValue >> 3;

    //invert CC data if potInverted is true
    if (getAnalogInvertState(analogID))   ccValue = 127 - ccValue;

    //only send data if function isn't called in setup
    if (sendControlChangeCallback != NULL)  {

        //only use map when cc limits are different from defaults
        if ((analogLowerLimit[analogID] != 0) || (analogUpperLimit[analogID] != 127))
            sendControlChangeCallback(ccNumber[analogID], map_uint8(ccValue, 0, 127, analogLowerLimit[analogID], analogUpperLimit[analogID]), _CCchannel);

        else    sendControlChangeCallback(ccNumber[analogID], ccValue, _CCchannel);

    }

    //update values
    lastAnalogueValue[analogID] = tempValue;

}

bool OpenDeck::getAnalogEnabled(uint8_t analogID) {

    uint8_t arrayIndex = analogID/8;
    uint8_t analogIndex = analogID - 8*arrayIndex;

    return bitRead(analogEnabled[arrayIndex], analogIndex);

}

uint8_t OpenDeck::getAnalogType(uint8_t analogID) {

    return analogType[analogID];

}

bool OpenDeck::getAnalogInvertState(uint8_t analogID) {

    uint8_t arrayIndex = analogID/8;
    uint8_t analogIndex = analogID - 8*arrayIndex;

    return bitRead(analogInverted[arrayIndex], analogIndex);

}

uint8_t OpenDeck::getCCnumber(uint8_t analogID)    {

    return ccNumber[analogID];

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