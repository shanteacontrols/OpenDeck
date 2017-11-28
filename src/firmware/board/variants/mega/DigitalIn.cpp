#ifdef BOARD_A_MEGA

#include "Board.h"
#include "Variables.h"

bool Board::digitalInputDataAvailable()
{
    return (digitalInBufferCounter == DIGITAL_BUFFER_SIZE);
}

void Board::continueDigitalInReadout()
{
    digitalInBufferCounter = 0;
}

#endif