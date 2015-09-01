/*

OpenDECK library v1.3
File: Potentiometers.cpp
Last revision date: 2014-12-25
Author: Igor Petrovic

*/

#include "OpenDeck.h"

#define NUMBER_OF_SAMPLES 3
#define ADC_8_BIT_ENABLED bitRead(ADMUX, ADLAR)

//potentiometer must exceed this value before sending new value
const uint8_t midiCCstep = ADC_8_BIT_ENABLED ? 2 : 8;

uint8_t mapAnalog(uint8_t x, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max)    {

    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

}


void OpenDeck::readAnalog()   {

    uint8_t availableAnalogData = boardObject.analogDataAvailable();
    if (!availableAnalogData) return;

    int16_t analogData;
    uint8_t analogID = 0;

    //check values
    for (int i=0; i<availableAnalogData; i++)    {

        analogData = boardObject.getAnalogValue(i);
        analogID = boardObject.getAnalogID(i);
        bool analogIDenabled = getAnalogEnabled(analogID);

        if (analogIDenabled)   {

            bool readingDifferent = checkAnalogValueDifference(analogData, analogID);

            if (readingDifferent)   addAnalogSample(analogID, analogData);
            if (analogValueSampled(analogID))    {

                analogData = getMedianValue(analogID);
                processAnalogReading(analogData, analogID);

            }

        }

    }

}

bool OpenDeck::checkAnalogValueDifference(int16_t tempValue, uint8_t analogID)  {

    //calculate difference between current and previous reading
    int16_t analogDiff = tempValue - lastAnalogueValue[analogID];

    //get absolute difference
    if (analogDiff < 0)   analogDiff *= -1;

    return (analogDiff >= midiCCstep);

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

void OpenDeck::processAnalogReading(int16_t tempValue, uint8_t analogID)  {

    uint8_t ccValue = tempValue >> 3;

    //invert CC data if potInverted is true
    if (getAnalogInvertState(analogID))   ccValue = 127 - ccValue;

    //only use map when cc limits are different from defaults
    if ((analogLowerLimit[analogID] != 0) || (analogUpperLimit[analogID] != 127))
        sendControlChange(ccNumber[analogID], mapAnalog(ccValue, 0, 127, analogLowerLimit[analogID], analogUpperLimit[analogID]), _CCchannel);

    else sendControlChange(ccNumber[analogID], ccValue, _CCchannel);

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

void OpenDeck::sendControlChange(uint8_t ccNumber, uint8_t ccValue, uint8_t channel) {

    #ifdef USBMIDI
        usbMIDI.sendControlChange(ccNumber, ccValue, channel);
    #endif

    #ifdef HW_MIDI
        MIDI.sendControlChange(ccNumber, ccValue, channel);
    #endif

}