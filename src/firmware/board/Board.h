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

typedef enum
{
    BOARD_OPEN_DECK_ID,
    BOARD_A_LEO_ID,
    BOARD_A_MEGA_ID
} boardID_t;

#ifdef BOARD_OPEN_DECK
#include "avr/variants/opendeck/Board.h"
#define BOARD_ID    BOARD_OPEN_DECK_ID
#elif defined(BOARD_A_LEO)
#include "avr/variants/leonardo/Board.h"
#define BOARD_ID    BOARD_A_LEO_ID
#elif defined(BOARD_A_MEGA)
#include "avr/variants/mega/Board.h"
#define BOARD_ID    BOARD_A_MEGA_ID
#elif defined(BOARD_A_16u2)
#include "avr/variants/16u2/Board.h"
//no id needed
#endif