#pragma once

#include "../core/src/Core.h"
#include "../midi/src/MIDI.h"
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