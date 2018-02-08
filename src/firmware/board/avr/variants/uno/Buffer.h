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

#include "Hardware.h"

#define ANALOG_BUFFER_SIZE  MAX_NUMBER_OF_ANALOG
#if ((MAX_NUMBER_OF_BUTTONS % 8) != 0)
#define DIGITAL_BUFFER_SIZE ((MAX_NUMBER_OF_BUTTONS / 8) + 1)
#define BUTTON_INDEX_SUBST  ((MAX_NUMBER_OF_BUTTONS % 8) - 1)
#else
#define DIGITAL_BUFFER_SIZE (MAX_NUMBER_OF_BUTTONS / 8)
#define BUTTON_INDEX_SUBST  7
#endif
