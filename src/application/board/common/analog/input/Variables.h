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

#include "Hardware.h"

#ifdef USE_MUX
///
/// \brief Holds currently active multiplexer.
///
extern uint8_t              activeMux;

///
/// \brief Holds currently active multiplexer input.
///
extern uint8_t              activeMuxInput;
#endif

///
/// \brief Holds currently active analog index which is being read.
/// Once all analog inputs are read, analog index is reset to 0.
///
extern uint8_t              analogIndex;

///
/// brief Holds currently active sample count.
/// Once all analog inputs are read, sample count is increased.
///
extern volatile uint8_t     analogSampleCounter;

///
/// \brief Array in which analog samples are stored.
///
extern volatile int16_t     analogBuffer[MAX_NUMBER_OF_ANALOG];