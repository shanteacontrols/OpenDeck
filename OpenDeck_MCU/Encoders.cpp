/*

OpenDECK library v1.3
File: Buttons.cpp
Last revision date: 2014-12-25
Author: Igor Petrovic

*/

#include "OpenDeck.h"

#define ENCODER_FAST_MODE_STABLE_AFTER  1
#define ENCODER_FAST_MODE_DEBOUNCE_TIME 60

void OpenDeck::readEncoders()   {

    for (int i=0; i<NUMBER_OF_ENCODERS; i++)    {

        if (!getEncoderEnabled(i)) continue;

        encoderPosition encoderState = boardObject.getEncoderState(i);
        if (encoderState == encStopped) continue;

        if (getEncoderInvertState(i))   {

            if (encoderState == encMoveLeft)
                encoderState = encMoveRight;

             else encoderState = encMoveLeft;

        } sendControlChange(encoderNumber[i], encoderState, _CCchannel);

    }

}

bool OpenDeck::getEncoderEnabled(uint8_t encoderID) {

    uint8_t arrayIndex = encoderID >> 3;
    uint8_t encoderIndex = encoderID - 8*arrayIndex;

    return bitRead(encoderEnabled[arrayIndex], encoderIndex);

}

bool OpenDeck::getEncoderInvertState(uint8_t encoderID) {

    uint8_t arrayIndex = encoderID >> 3;
    uint8_t encoderIndex = encoderID - 8*arrayIndex;

    return bitRead(encoderInverted[arrayIndex], encoderIndex);

}

bool OpenDeck::getEncoderFastMode(uint8_t encoderID)  {

    uint8_t arrayIndex = encoderID >> 3;
    uint8_t encoderIndex = encoderID - 8*arrayIndex;

    return bitRead(encoderFastMode[arrayIndex], encoderIndex);

}

void OpenDeck::resetEncoderValues(uint8_t encoderID) {

    initialEncoderDebounceCounter[encoderID] = 0;
    lastEncoderSpinTime[encoderID] = 0;

}