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
/// \brief Provides helper functions to facilitate software reset on MCU.
/// \defgroup reset Reset
/// \ingroup core
/// @{

///
/// \brief Disables all peripherals present on MCU.
///
void disablePeripherals();

///
/// \brief Initiates watchdog software MCU reset by setting watch-dog timeout and waiting until watchdog is activated.
///
void wdReset();

/// @}