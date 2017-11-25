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

#ifdef BOARD_OPEN_DECK

///
/// \brief Hardcoded board version.
/// @{
///

#define HARDWARE_VERSION_MAJOR      1
#define HARDWARE_VERSION_MINOR      2
#define HARDWARE_VERSION_REVISION   0

/// @}

#include "variants/opendeck//Board.h"
#elif defined(BOARD_A_LEO)

///
/// \brief Hardcoded board version.
/// @{
///

#define HARDWARE_VERSION_MAJOR      1
#define HARDWARE_VERSION_MINOR      0
#define HARDWARE_VERSION_REVISION   0

#include "variants/leonardo/Leonardo.h"

/// @}

#elif defined(BOARD_A_MEGA)
#define HARDWARE_VERSION_MAJOR      1
#define HARDWARE_VERSION_MINOR      0
#define HARDWARE_VERSION_REVISION   0

#include "variants/mega/Board.h"

#endif