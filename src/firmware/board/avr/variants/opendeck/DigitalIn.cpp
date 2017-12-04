#include "Board.h"
#include "Variables.h"

volatile uint8_t    digitalInBuffer[NUMBER_OF_BUTTON_COLUMNS];
volatile uint8_t    digitalInBuffer_copy[NUMBER_OF_BUTTON_COLUMNS];
volatile uint8_t    activeInColumn;

bool Board::digitalInputDataAvailable()
{
    return (activeInColumn == NUMBER_OF_BUTTON_COLUMNS);
}

void Board::continueDigitalInReadout()
{
    activeInColumn = 0;
}
