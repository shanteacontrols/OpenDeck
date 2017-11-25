#include "DigitalInput.h"

DigitalInput::DigitalInput()
{
    
}

void DigitalInput::update()
{
    if (!board.digitalInputDataAvailable())
    {
        return;
    }
    else
    {
        buttons.update();
        encoders.update();
        board.continueDigitalInReadout();
    }
}

DigitalInput digitalInput;