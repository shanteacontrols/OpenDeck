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
#include "Hardware.h"

///
/// \brief Holds value of currently active LED matrix column.
///
volatile uint8_t    activeOutColumn;

///
/// \brief Used to calculate index of R, G or B component of RGB LED.
/// @param [in] rgbID   Index of RGB LED.
/// @param [in] index   R, G or B component (enumerated type, see rgbIndex_t).
/// \returns Calculated index of R, G or B component of RGB LED.
///
uint8_t Board::getRGBaddress(uint8_t rgbID, rgbIndex_t index)
{
    uint8_t column = rgbID % NUMBER_OF_LED_COLUMNS;
    uint8_t row  = (rgbID/NUMBER_OF_BUTTON_COLUMNS)*3;
    uint8_t address = column + NUMBER_OF_LED_COLUMNS*row;

    switch(index)
    {
        case rgb_R:
        return address;

        case rgb_G:
        return address + NUMBER_OF_LED_COLUMNS*1;

        case rgb_B:
        return address + NUMBER_OF_LED_COLUMNS*2;
    }

    return 0;
}

///
/// \brief Calculates RGB LED index based on provided single-color LED index.
/// @param [in] ledID   Index of single-color LED.
/// \returns Calculated index of RGB LED.
///
uint8_t Board::getRGBID(uint8_t ledID)
{
    uint8_t row = ledID/NUMBER_OF_LED_COLUMNS;

    uint8_t mod = row%3;    //RGB LED = 3 normal LEDs
    row -= mod;

    uint8_t column = ledID % NUMBER_OF_BUTTON_COLUMNS;

    return (row*NUMBER_OF_LED_COLUMNS)/3 + column;
}
