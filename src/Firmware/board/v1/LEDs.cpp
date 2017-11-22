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

volatile bool       blinkEnabled,
                    blinkState;

volatile uint8_t    pwmSteps,
                    ledState[MAX_NUMBER_OF_LEDS],
                    activeLEDcolumn;

volatile uint16_t   ledBlinkTime;

volatile int8_t     transitionCounter[MAX_NUMBER_OF_LEDS];
volatile uint32_t   blinkTimerCounter;


uint8_t Board::getRGBaddress(uint8_t rgbID, rgbIndex_t index)
{
    //get RGB LED address for specified index
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

uint8_t Board::getRGBID(uint8_t ledNumber)
{
    uint8_t row = ledNumber/NUMBER_OF_LED_COLUMNS;

    uint8_t mod = row%3;    //RGB LED = 3 normal LEDs
    row -= mod;

    uint8_t column = ledNumber % NUMBER_OF_BUTTON_COLUMNS;

    return (row*NUMBER_OF_LED_COLUMNS)/3 + column;
}

#endif