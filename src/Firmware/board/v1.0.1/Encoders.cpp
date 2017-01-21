#include "Board.h"
#include "Variables.h"

bool                encodersProcessed;
uint16_t            encoderData[MAX_NUMBER_OF_ENCODERS];

void Board::initEncoders()
{
    for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
    {
        encoderData[i] |= ((uint16_t)0 << 8);
        encoderData[i] |= ((uint16_t)ENCODER_DEFAULT_PULSE_COUNT_STATE << 4);   //set number of pulses to 8
    }
}

uint8_t Board::getEncoderPair(uint8_t buttonIndex)
{
    uint8_t row = buttonIndex/NUMBER_OF_BUTTON_COLUMNS;

    if (row%2)
        row -= 1;   //uneven row, get info from previous (even) row

    uint8_t column = buttonIndex % NUMBER_OF_BUTTON_COLUMNS;
    return (row*NUMBER_OF_BUTTON_COLUMNS)/2 + column;
}

bool Board::encoderDataAvailable()
{
    checkInputMatrixBufferCopy();

    bool returnValue = true;
    bool _dmBufferCopied;
    _dmBufferCopied = dmBufferCopied;

    if (!_dmBufferCopied)
        returnValue = copyInputMatrixBuffer();  //buffer isn't copied

    encodersProcessed = true;
    return returnValue;
}

int8_t Board::getEncoderState(uint8_t encoderNumber)
{
    uint8_t column = encoderNumber % NUMBER_OF_BUTTON_COLUMNS;
    uint8_t row  = (encoderNumber/NUMBER_OF_BUTTON_COLUMNS)*2;
    uint8_t shiftAmount = ((NUMBER_OF_BUTTON_COLUMNS-1)*8) - column*8;
    uint8_t pairState = inputMatrixBufferCopy >> shiftAmount;
    pairState = ((pairState >> row) & 0x03);

    return readEncoder(encoderNumber, pairState);
}

inline int8_t readEncoder(uint8_t encoderID, uint8_t pairState)
{
    //add new data
    uint8_t newPairData = 0;
    newPairData |= (((encoderData[encoderID] << 2) & 0x000F) | (uint16_t)pairState);

    //remove old data
    encoderData[encoderID] &= ENCODER_CLEAR_TEMP_STATE_MASK;

    //shift in new data
    encoderData[encoderID] |= (uint16_t)newPairData;

    int8_t encRead = encoderLookUpTable[newPairData];

    if (!encRead)
        return 0;

    bool newEncoderDirection = encRead > 0;
    //get current number of pulses from encoderData
    int8_t currentPulses = (encoderData[encoderID] >> 4) & 0x000F;
    currentPulses += encRead;
    //clear current pulses
    encoderData[encoderID] &= ENCODER_CLEAR_PULSES_MASK;
    //shift in new pulse count
    encoderData[encoderID] |= (uint16_t)(currentPulses << 4);
    //get last encoder direction
    bool lastEncoderDirection = bitRead(encoderData[encoderID], ENCODER_DIRECTION_BIT);
    //write new encoder direction
    bitWrite(encoderData[encoderID], ENCODER_DIRECTION_BIT, newEncoderDirection);

    if (lastEncoderDirection != newEncoderDirection)
        return 0;

    if (currentPulses % PULSES_PER_STEP)
        return 0;

    //clear current pulses
    encoderData[encoderID] &= ENCODER_CLEAR_PULSES_MASK;

    //set default pulse count
    encoderData[encoderID] |= ((uint16_t)ENCODER_DEFAULT_PULSE_COUNT_STATE << 4);

    if (newEncoderDirection)
        return 1;
    else
        return -1;
}
