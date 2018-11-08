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

#include "Global.h"
#include "Buttons.h"
#include "Encoders.h"
#include "Analog.h"
#include "LEDs.h"
#include "Display.h"

//define block names
enum sysExBlocks
{
    SYSEX_BLOCK_GLOBAL,    //0
    SYSEX_BLOCK_BUTTONS,   //1
    SYSEX_BLOCK_ENCODERS,  //2
    SYSEX_BLOCK_ANALOG,    //3
    SYSEX_BLOCK_LEDS,      //4
    SYSEX_BLOCK_DISPLAY,   //5
    SYSEX_BLOCKS
};
