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
/// \brief Array used to store readings from digital input matrix.
///
volatile uint8_t    digitalInBuffer[NUMBER_OF_BUTTON_COLUMNS];

///
/// \brief Holds value of currently active input matrix column.
///
volatile uint8_t    activeInColumn;

///
/// \brief Checks if data from button matrix is available.
/// Matrix data is read in ISR and stored into digitalInBuffer array.
/// Once all columns are read, data is considered available.
/// At this point, input matrix column variable is set to invalid value
/// to stop further data reading from ISR until continueDigitalInReadout
/// function is called.
/// \returns True if data is available, false otherwise.
///
bool Board::digitalInputDataAvailable()
{
    return (activeInColumn == NUMBER_OF_BUTTON_COLUMNS);
}

///
/// \brief Resets active input matrix column so that readings in ISR can continue.
///
void Board::continueDigitalInReadout()
{
    activeInColumn = 0;
}
