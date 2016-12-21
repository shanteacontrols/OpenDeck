#include "Board.h"
#include "Variables.h"

bool buttonsProcessed = false;

bool Board::buttonDataAvailable()
{
    checkInputMatrixBufferCopy();

    bool returnValue = true;
    bool _dmBufferCopied;
    _dmBufferCopied = dmBufferCopied;

    if (!_dmBufferCopied)
        returnValue = copyInputMatrixBuffer();  //buffer isn't copied

    buttonsProcessed = true;
    return returnValue;
}

bool Board::getButtonState(uint8_t buttonIndex)
{
    uint8_t row = buttonIndex/NUMBER_OF_BUTTON_COLUMNS;
    //invert column order
    uint8_t column = (NUMBER_OF_BUTTON_COLUMNS-1) - buttonIndex % NUMBER_OF_BUTTON_COLUMNS;
    buttonIndex = column*8 + row;

    return !((inputMatrixBufferCopy >> buttonIndex) & 0x01);
}