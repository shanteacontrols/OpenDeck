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