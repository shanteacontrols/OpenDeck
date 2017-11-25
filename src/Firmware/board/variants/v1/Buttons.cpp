/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2017 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef BOARD_OPEN_DECK

#include "Board.h"
#include "Variables.h"

bool buttonsProcessed;

bool Board::buttonDataAvailable()
{
    checkInputMatrixBufferCopy();

    bool returnValue = true;
    bool _dmBufferCopied = dmBufferCopied;

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

#endif