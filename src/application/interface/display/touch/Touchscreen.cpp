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
#include "model/sdw/SDW.h"

///
/// \brief Initializes specified touchscreen.
/// @param [in] touchscreenType Touchscreen type. See ts_t.
/// \returns True on success, false otherwise.
///
bool Touchscreen::init(ts_t touchscreenType)
{
    switch(touchscreenType)
    {
        case ts_sdw:
        sdw_init(*this);
        return true;

        default:
        return false;
    }
}

///
/// \brief Checks for incoming data from display.
///
void Touchscreen::update()
{
    if (displayUpdatePtr == nullptr)
        return;

    if ((*displayUpdatePtr)(*this))
    {
        if (buttonHandler != nullptr)
            (*buttonHandler)(activeButtonID, activeButtonState);
    }
}

///
/// \brief Switches to requested page on display
/// @param [in] pageID  Index of page to display.
///
void Touchscreen::setPage(uint8_t pageID)
{
    if (setPagePtr == nullptr)
        return;

    (*setPagePtr)(pageID);
    activePage = pageID;
}

///
/// \brief Used to retrieve currently active page on display.
/// \returns Active display page.
///
uint8_t Touchscreen::getPage()
{
    return activePage;
}

///
/// \param fptr [in]    Pointer to function.
///
void Touchscreen::setButtonHandler(void(*fptr)(uint8_t index, bool state))
{
    buttonHandler = fptr;
}