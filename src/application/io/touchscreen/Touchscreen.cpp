/*

Copyright 2015-2020 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "Touchscreen.h"

using namespace IO;

bool Touchscreen::init()
{
    initialized = model.init();
    return initialized;
}

void Touchscreen::update()
{
    if (!initialized)
        return;

    size_t buttonID = 0;
    bool   state    = false;

    if (model.update(buttonID, state))
    {
        if (isScreenChangeButton(buttonID, activeScreenID))
        {
            if (screenHandler != nullptr)
                screenHandler(activeScreenID);
        }

        if (buttonHandler != nullptr)
            (*buttonHandler)(buttonID, state);
    }
}

///
/// \brief Switches to requested screen on display
/// @param [in] screenID  Index of screen to display.
///
void Touchscreen::setScreen(size_t screenID)
{
    model.setScreen(screenID);
    activeScreenID = screenID;

    if (screenHandler != nullptr)
        screenHandler(screenID);
}

///
/// \brief Used to retrieve currently active screen on display.
/// \returns Active display screen.
///
size_t Touchscreen::activeScreen()
{
    return activeScreenID;
}

void Touchscreen::setButtonHandler(void (*fptr)(size_t index, bool state))
{
    buttonHandler = fptr;
}

void Touchscreen::setScreenChangeHandler(void (*fptr)(size_t screenID))
{
    screenHandler = fptr;
}

void Touchscreen::setIconState(size_t index, bool state)
{
    icon_t icon;

    if (!getIcon(index, icon))
        return;

    if ((activeScreenID != icon.onScreen) && (activeScreenID != icon.offScreen))
        return;    //don't allow setting icon on wrong screen

    model.setIconState(icon, state);
}

__attribute__((weak)) bool Touchscreen::getIcon(size_t index, icon_t& icon)
{
    return false;
}

__attribute__((weak)) bool Touchscreen::isScreenChangeButton(size_t index, size_t& screenID)
{
    return false;
}