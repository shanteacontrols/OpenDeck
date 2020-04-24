#include "Nextion.h"

bool Nextion::init()
{
    return true;
}

void Nextion::setScreen(uint8_t screenID)
{
}

bool Nextion::update(uint8_t& buttonID, bool& state)
{
    return false;
}

void Nextion::setButtonState(uint8_t index, bool state)
{
    icon_t icon;

    if (!getIcon(index, icon))
        return;
}

__attribute__((weak)) bool Nextion::getIcon(size_t index, icon_t& icon)
{
    return false;
}
