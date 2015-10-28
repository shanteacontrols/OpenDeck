/*

OpenDECK library v1.3
File: Potentiometers.cpp
Last revision date: 2014-12-25
Author: Igor Petrovic

*/

#include "OpenDeck.h"

#define NUMBER_OF_SAMPLES 3

void OpenDeck::readAnalog()   {

    uint8_t availableAnalogData = boardObject.analogDataAvailable();
    if (!availableAnalogData) return;

    int16_t analogData;
    uint8_t analogID = 0;

    //check values
    for (int i=0; i<availableAnalogData; i++)    {

        analogData = boardObject.getAnalogValue(i); //get raw analog reading
        analogID = boardObject.getAnalogID(i);  //get hardware ID for analog component
        if (!getAnalogEnabled(analogID)) continue; //don't process component if it's not enabled
        addAnalogSample(analogID, analogData);  //add current reading to sample array
        if (!analogValueSampled(analogID)) continue;  //three samples are needed
        analogData = getMedianValue(analogID);  //get median value from three analog samples for better accuracy
        analogType type = getAnalogType(analogID);  //get type of current analog component

        switch(type) {

            case potentiometer:
            checkPotentiometerValue(analogData, analogID);
            break;

            case fsr1:
            case fsr2:
            case fsr3:
            checkFSRvalue(analogData, analogID, type);
            break;

            case ldr:
            break;

            default:
            return;

        }

    }

}

void OpenDeck::addAnalogSample(uint8_t analogID, int16_t sample) {

    uint8_t sampleIndex = analogDebounceCounter[analogID];

    analogSample[analogID][sampleIndex] = sample;
    analogDebounceCounter[analogID]++;

}

bool OpenDeck::analogValueSampled(uint8_t analogID) {

    if (analogDebounceCounter[analogID] == NUMBER_OF_SAMPLES) {

        analogDebounceCounter[analogID] = 0;
        return true;

    }   return false;

}

bool OpenDeck::getAnalogEnabled(uint8_t analogID) {

    uint8_t arrayIndex = analogID/8;
    uint8_t analogIndex = analogID - 8*arrayIndex;

    return bitRead(analogEnabled[arrayIndex], analogIndex);

}

analogType OpenDeck::getAnalogType(uint8_t analogID) {

    return _analogType[analogID];

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

uint8_t OpenDeck::mapAnalog(uint8_t x, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max)    {

    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

}

void OpenDeck::sendControlChange(uint8_t ccNumber, uint8_t ccValue, uint8_t channel) {

    #ifdef USBMIDI
        usbMIDI.sendControlChange(ccNumber, ccValue, channel);
    #endif

    #ifdef HW_MIDI
        MIDI.sendControlChange(ccNumber, ccValue, channel);
    #endif

}