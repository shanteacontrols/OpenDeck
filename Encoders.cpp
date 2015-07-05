/*

OpenDECK library v1.3
File: Buttons.cpp
Last revision date: 2014-12-25
Author: Igor Petrovic

*/

#include "OpenDeck.h"
#include "Ownduino.h"

#define ENCODER_FAST_MODE_STABLE_AFTER  1
#define ENCODER_FAST_MODE_DEBOUNCE_TIME 60

#define ENCODER_VALUE_LEFT              1
#define ENCODER_VALUE_RIGHT             127


//void OpenDeck::processEncoderPair(uint8_t encoderPair, uint8_t columnState, uint8_t row)    {
//
    //uint8_t newValues = 0;
//
    //switch (row)    {
//
        //case 0:
        //case 1:
        //newValues = (columnState >> 0) & 0x03;
        //break;
//
        //case 2:
        //case 3:
        //newValues = (columnState >> 2) & 0x03;
        //break;
//
        //case 4:
        //case 5:
        //newValues = (columnState >> 4) & 0x03;
        //break;
//
        //case 6:
        //case 7:
        //newValues = (columnState >> 6) & 0x03;
        //break;
//
        //default:
        //break;
//
    //}
//
    ////shift last state two bytes left to make room for new one
    //encoderPairState[encoderPair] <<= 2;
    ////insert new values
    //encoderPairState[encoderPair] |= newValues;
//
    //int8_t direction = enc_states[encoderPairState[encoderPair] & 0x0f];
//
    //switch (direction)  {
//
        //case -1:
        ////do something
        //break;
//
        //case 1:
        ////do something
        //break;
//
        //default:
        //break;
//
    //}
//
//}

uint8_t OpenDeck::getEncoderPairNumber(uint8_t row, uint8_t buttonNumber)   {

    //check for whether row is even or not to calculate encoder pair
    //uint8_t rowEven = (row % 2 == 0);
    //get encoder pair number based on current row and buttonNumber
    //return (buttonNumber - _numberOfColumns*!rowEven) - (row-1*!rowEven)*_numberOfButtonRows;
    return 0;

}

void OpenDeck::readEncoders()   {

    for (int i=0; i<NUMBER_OF_ENCODERS; i++)   {

        if (getEncoderEnabled(i))   {

            int32_t encState = boardObject.getEncoderState(i);

            if (encState != lastEncoderState[i])    {  //encoder is moving

                if (millisOwnduino() - lastEncoderSpinTime[i] > ENCODER_FAST_MODE_DEBOUNCE_TIME)
                    initialEncoderDebounceCounter[i] = 0;

                if (encState > lastEncoderState[i]) {

                    if (!encoderDirection[i]) {

                        initialEncoderDebounceCounter[i] = 0;
                        encoderDirection[i] = true;
                        pulseCounter[i] = 0;

                    }

                    //only do additional debouncing if encoder is set to fast mode
                    if ((initialEncoderDebounceCounter[i] >= ENCODER_FAST_MODE_STABLE_AFTER) || (!getEncoderFastMode(i)))   {

                        pulseCounter[i]++;

                        if (pulseCounter[i] >= pulsesPerStep[i])   {

                            if (getEncoderInvertState(i))
                                sendControlChangeCallback(encoderNumber[i], ENCODER_DIRECTION_LEFT, _analogCCchannel);

                            else sendControlChangeCallback(encoderNumber[i], ENCODER_DIRECTION_RIGHT, _analogCCchannel);

                            pulseCounter[i] = 0;

                        }

                    }   else initialEncoderDebounceCounter[i]++;

                }

                else if (encState < lastEncoderState[i]) {

                    if (encoderDirection[i])    {

                        initialEncoderDebounceCounter[i] = 0;
                        encoderDirection[i] = false;
                        pulseCounter[i] = 0;

                    }

                    if ((initialEncoderDebounceCounter[i] >= ENCODER_FAST_MODE_STABLE_AFTER) || (!getEncoderFastMode(i)))   {

                        pulseCounter[i]++;

                        if (pulseCounter[i] >= pulsesPerStep[i])   {

                            if (getEncoderInvertState(i))
                                sendControlChangeCallback(encoderNumber[i], ENCODER_DIRECTION_RIGHT, _analogCCchannel);

                            else sendControlChangeCallback(encoderNumber[i], ENCODER_DIRECTION_LEFT, _analogCCchannel);

                            pulseCounter[i] = 0;

                        }

                    }   else initialEncoderDebounceCounter[i]++;

                }

                lastEncoderState[i] = encState;
                lastEncoderSpinTime[i] = millisOwnduino();

            }

        }

        }

}

bool OpenDeck::getEncoderEnabled(uint8_t encoder) {

    uint8_t arrayIndex = encoder/8;
    uint8_t encoderIndex = encoder - 8*arrayIndex;

    return bitRead(encoderEnabled[arrayIndex], encoderIndex);

}

bool OpenDeck::getEncoderInvertState(uint8_t encoder) {

    uint8_t arrayIndex = encoder/8;
    uint8_t encoderIndex = encoder - 8*arrayIndex;

    return bitRead(encoderInverted[arrayIndex], encoderIndex);

}

bool OpenDeck::getEncoderFastMode(uint8_t encoder)  {

    uint8_t arrayIndex = encoder/8;
    uint8_t encoderIndex = encoder - 8*arrayIndex;

    return bitRead(encoderFastMode[arrayIndex], encoderIndex);

}

void OpenDeck::resetEncoderValues(uint8_t encoder) {

    initialEncoderDebounceCounter[encoder] = 0;
    pulseCounter[encoder] = 0;
    lastEncoderSpinTime[encoder] = 0;

}