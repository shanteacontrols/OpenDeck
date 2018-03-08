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

#pragma once

#include "MIDI.h"
#include "Buttons.h"
#include "Encoders.h"
#include "Analog.h"
#include "LEDs.h"
#include "Display.h"
#include "ID.h"

//define block names
enum dbBlocks
{
    DB_BLOCK_MIDI,      //0
    DB_BLOCK_BUTTONS,   //1
    DB_BLOCK_ENCODERS,  //2
    DB_BLOCK_ANALOG,    //3
    DB_BLOCK_LEDS,      //4
    DB_BLOCK_DISPLAY,   //5
    DB_BLOCK_ID,        //6
    DB_BLOCKS
};
