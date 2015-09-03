/*

OpenDECK library v1.3
File: Buttons.cpp
Last revision date: 2014-12-25
Author: Igor Petrovic

*/

#include "OpenDeck.h"

#define ENCODER_VALUE_LEFT_7FH01H   127
#define ENCODER_VALUE_RIGHT_7FH01H  1

#define ENCODER_VALUE_LEFT_3FH41H   63
#define ENCODER_VALUE_RIGHT_3FH41H  65

void OpenDeck::readEncoders()   {

    for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)    {

        if (!getEncoderEnabled(i)) continue;

        encoderPosition encoderState = boardObject.getEncoderState(i);
        if (encoderState == encStopped) continue;

        if (getEncoderInvertState(i))   {

            if (encoderState == encMoveLeft)
                encoderState = encMoveRight;

             else encoderState = encMoveLeft;

        }

        uint8_t encoderValue = 0;

        switch(_encoderType[i]) {

            case enc7Fh01h:
            if (encoderState == encMoveLeft) encoderValue = ENCODER_VALUE_LEFT_7FH01H;
            else encoderValue = ENCODER_VALUE_RIGHT_7FH01H;
            break;

            case enc3Fh41h:
            if (encoderState == encMoveLeft) encoderValue = ENCODER_VALUE_LEFT_3FH41H;
            else encoderValue = ENCODER_VALUE_RIGHT_3FH41H;
            break;

        }

        sendControlChange(encoderNumber[i], encoderValue, _CCchannel);

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

encoderType OpenDeck::getEncoderType(uint8_t encoderID)  {

    return _encoderType[encoderID];

}

void OpenDeck::resetEncoderValues(uint8_t encoderID) {

    initialEncoderDebounceCounter[encoderID] = 0;
    lastEncoderSpinTime[encoderID] = 0;

}