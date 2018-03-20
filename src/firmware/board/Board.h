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

#ifndef _BOARD_
#define _BOARD_

typedef enum
{
    BOARD_OPEN_DECK_ID,
    BOARD_A_LEO_ID,
    BOARD_A_MEGA_ID,
    BOARD_A_PRO_MICRO_ID,
    BOARD_A_UNO_ID,
    BOARD_T_2PP_ID
} boardID_t;

#ifdef BOARD_OPEN_DECK
#include "avr/variants/opendeck/Board.h"
#define BOARD_ID    BOARD_OPEN_DECK_ID
#elif defined(BOARD_A_LEO)
#include "avr/variants/leonardo/Board.h"
#define BOARD_ID    BOARD_A_LEO_ID
#elif defined(BOARD_A_PRO_MICRO)
#include "avr/variants/leonardo/Board.h"
#define BOARD_ID    BOARD_A_PRO_MICRO_ID
#elif defined(BOARD_A_MEGA)
#include "avr/variants/mega/Board.h"
#define BOARD_ID    BOARD_A_MEGA_ID
#elif defined(BOARD_A_UNO)
#include "avr/variants/uno/Board.h"
#define BOARD_ID    BOARD_A_UNO_ID
#elif defined(BOARD_T_2PP)
#include "avr/variants/teensy2pp/Board.h"
#define BOARD_ID    BOARD_T_2PP_ID
#elif defined(BOARD_A_xu2)
#include "avr/variants/xu2/Board.h"
//no id needed
#endif
#endif