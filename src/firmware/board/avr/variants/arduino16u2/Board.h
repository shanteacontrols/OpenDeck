#pragma once

#include "../../../common/Common.h"
#include "Pins.h"
#include "Variables.h"

class Board
{
    public:
    Board();
    void init();

    private:
    void initUART_MIDI();
    void initUSB_MIDI();
    void initPins();
};