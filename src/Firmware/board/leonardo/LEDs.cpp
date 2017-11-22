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

#ifdef BOARD_A_LEO

#include "Board.h"

volatile bool       blinkEnabled,
                    blinkState;

volatile uint8_t    pwmSteps,
                    ledState[MAX_NUMBER_OF_LEDS];

volatile uint16_t   ledBlinkTime;
volatile uint32_t   blinkTimerCounter;


uint8_t Board::getRGBaddress(uint8_t rgbID, rgbIndex_t index)
{
    //get RGB LED address for specified index
    return rgbID*3+(uint8_t)index;
}

uint8_t Board::getRGBID(uint8_t ledNumber)
{
    return ledNumber / 3;    //RGB LED = 3 normal LEDs
}

#endif