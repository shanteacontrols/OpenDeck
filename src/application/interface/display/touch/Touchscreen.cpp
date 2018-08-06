/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

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

#include "Touchscreen.h"
#include "Variables.h"
#include "model/sdw/SDW.h"
#include "interface/digital/input/buttons/Buttons.h"


bool        (*displayUpdatePtr)();
void        (*setPagePtr)(uint8_t pageID);
uint8_t     displayRxBuffer[TOUCHSCREEN_RX_BUFFER_SIZE];
uint8_t     bufferIndex_rx;
uint8_t     activeButtonID;
bool        activeButtonState;
uint8_t     activePage;


///
/// \brief Default constructor.
///
Touchscreen::Touchscreen()
{
    displayUpdatePtr = NULL;
    setPagePtr = NULL;
}

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
        sdw_init();
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
    if (displayUpdatePtr == NULL)
        return;

    if ((*displayUpdatePtr)())
        process(MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG+activeButtonID, activeButtonState);
}

///
/// \brief Used to process touch event (button press).
/// @param [in] buttonID    Button index on display which was pressed.
/// @param [in] buttonState State of the button (true/pressed, false/released).
///
void Touchscreen::process(uint8_t buttonID, bool buttonState)
{
    buttons.processButton(activeButtonID, buttonState);
}

///
/// \brief Switches to requested page on display
/// @param [in] pageID  Index of page to display.
///
void Touchscreen::setPage(uint8_t pageID)
{
    if (setPagePtr == NULL)
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

Touchscreen touchscreen;