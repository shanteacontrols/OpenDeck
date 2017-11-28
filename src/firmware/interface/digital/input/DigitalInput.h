#pragma once

#include "buttons/Buttons.h"
#include "encoders/Encoders.h"

class DigitalInput
{
    public:
    DigitalInput();
    void update();
};

extern DigitalInput digitalInput;