/*

OpenDECK library v1.1
File: Buttons.cpp
Last revision date: 2014-11-02
Author: Igor Petrovic

*/

#include "OpenDeck.h"
#include "Ownduino.h"

//lookup table
static const int8_t enc_states [] =
{0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};


void OpenDeck::processEncoderPair(uint8_t encoderPair, uint8_t columnState, uint8_t row)    {

    uint8_t newValues = 0;

    switch (row)    {

        case 0:
        case 1:
        newValues = (columnState >> 0) & 0x03;
        break;

        case 2:
        case 3:
        newValues = (columnState >> 2) & 0x03;
        break;

        case 4:
        case 5:
        newValues = (columnState >> 4) & 0x03;
        break;

        case 6:
        case 7:
        newValues = (columnState >> 6) & 0x03;
        break;

        default:
        break;

    }

    //shift last state two bytes left to make room for new one
    encoderPairState[encoderPair] <<= 2;
    //insert new values
    encoderPairState[encoderPair] |= newValues;

    int8_t direction = enc_states[encoderPairState[encoderPair] & 0x0f];

    switch (direction)  {

        case -1:
        //do something
        break;

        case 1:
        //do something
        break;

        default:
        break;

    }

}

uint8_t OpenDeck::getEncoderPairNumber(uint8_t row, uint8_t buttonNumber)   {

    //check for whether row is even or not to calculate encoder pair
    uint8_t rowEven = (row % 2 == 0);
    //get encoder pair number based on current row and buttonNumber
    return (buttonNumber - _numberOfColumns*!rowEven) - (row-1*!rowEven)*_numberOfButtonRows;

}