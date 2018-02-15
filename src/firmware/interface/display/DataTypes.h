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
/// \brief List of all possible text types on display.
///
typedef enum
{
    lcdtext_still,
    lcdText_temp
} lcdTextType_t;

///
/// \brief List of all possible text scrolling directions.
///
typedef enum
{
    scroll_ltr,
    scroll_rtl
} scrollDirection_t;

///
/// \brief Structure holding data for scrolling event on display for single row.
///
typedef struct
{
    uint8_t size;
    uint8_t startIndex;
    int8_t currentIndex;
    scrollDirection_t direction;
} scrollEvent_t;
