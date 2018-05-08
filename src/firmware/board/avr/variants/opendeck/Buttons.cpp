/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

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

#include "Board.h"
#include "Variables.h"

///
/// \brief Returns last read button state for requested button index.
/// @param [in] buttonIndex Index of button which should be read.
/// \returns True if button is pressed, false otherwise.
///
bool Board::getButtonState(uint8_t buttonIndex)
{
    uint8_t row = buttonIndex/NUMBER_OF_BUTTON_COLUMNS;
    uint8_t column = buttonIndex % NUMBER_OF_BUTTON_COLUMNS;

    return BIT_READ(digitalInBuffer[column], row);
}
