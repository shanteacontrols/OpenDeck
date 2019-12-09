/*

Copyright 2015-2019 Igor Petrovic

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

using namespace Interface;

bool Touchscreen::init()
{
    return hwa.init();
}

void Touchscreen::update()
{
    uint8_t buttonID = 0;
    bool    state    = false;

    if (hwa.update(buttonID, state))
    {
        if (buttonHandler != nullptr)
            (*buttonHandler)(buttonID, state);
    }
}

///
/// \brief Switches to requested screen on display
/// @param [in] screenID  Index of screen to display.
///
void Touchscreen::setScreen(uint8_t screenID)
{
    hwa.setScreen(screenID);
    activeScreenID = screenID;
}

///
/// \brief Used to retrieve currently active screen on display.
/// \returns Active display screen.
///
uint8_t Touchscreen::activeScreen()
{
    return activeScreenID;
}

///
/// \param fptr [in]    Pointer to function.
///
void Touchscreen::setButtonHandler(void (*fptr)(uint8_t index, bool state))
{
    buttonHandler = fptr;
}