/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015, 2016 Igor Petrovic

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

#ifndef LED_COLORS_H_
#define LED_COLORS_H_

//some predefined colors here
#include "../../board/Board.h"

typedef enum {

    colorOff,
    colorWhite,
    colorCyan,
    colorMagenta,
    colorRed,
    colorBlue,
    colorYellow,
    colorGreen

} ledColor_t;

const rgb colors[] = {

    {   //off
        0,
        0,
        0
    },

    {   //white
        255,
        255,
        255
    },

    {   //cyan
        0,
        255,
        255
    },

    {   //magenta
        255,
        0,
        255
    },

    {   //red
        255,
        0,
        0
    },

    {   //blue
        0,
        0,
        255
    },

    {   //yellow
        255,
        255,
        0
    },

    {
        //green
        0,
        255,
        0
    }

};

#endif