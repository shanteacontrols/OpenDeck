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

#include "board/Board.h"
#include "Variables.h"

#ifdef OUT_MATRIX
///
/// \brief Holds value of currently active output matrix column.
///
volatile uint8_t    activeOutColumn;
#else
///
/// \brief Holds last LED state for all LEDs.
/// Used in direct-out setups only where LED state is updated only when it changes.
///
uint8_t             lastLEDstate[MAX_NUMBER_OF_LEDS];
#endif