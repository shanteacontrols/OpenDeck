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

#pragma once

///
/// \brief Provides delay function and interface to accommodate MCU run time in milliseconds.
/// \defgroup timer Timer
/// \ingroup core
/// @{

///
/// \brief Definition of variable holding current MCU run time. Must be implemented externally in order to
/// use rTimeMs() function correctly.
///
extern volatile uint32_t rTime_ms;

///
/// \brief Delays for desired time interval in milliseconds.
/// This function makes use of built-in _delay_ms function. Function is called repeatedly with argument 1 until
/// ms parameter reaches 0, since _delay_ms accepts only constant known at compile-time.
/// @param [in] ms  Delay time in milliseconds.
///
void wait_ms(uint32_t ms);

///
/// \brief Returns amount of time MCU has been running in milliseconds.
/// Actual incrementation of rTime_ms must be done externally.
/// \returns Runtime in milliseconds.
///
uint32_t rTimeMs();

/// @}