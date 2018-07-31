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


bool        (*displayUpdatePtr)();
void        (*setImagePtr)(uint8_t imageID);
void        (*setIconPtr)(uint16_t x, uint16_t y, uint16_t iconID);
uint8_t     displayRxBuffer[TOUCHSCREEN_RX_BUFFER_SIZE];
uint8_t     bufferIndex_rx;
uint8_t     activeButtonID;
bool        activeButtonState;

///
/// \brief Default constructor.
///
Touchscreen::Touchscreen()
{
    displayUpdatePtr = NULL;
    setImagePtr = NULL;
    setIconPtr = NULL;
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
        process(activeButtonID, activeButtonState);
}

///
/// \brief Used to process touch event (button press).
/// @param [in] buttonID    Button index on display which was pressed.
/// @param [in] buttonState State of the button (true/pressed, false/released).
///
void Touchscreen::process(uint8_t buttonID, bool buttonState)
{

}

///
/// \brief Displays full-screen image on display.
/// @param [in] imageID  Index of image to display.
///
void Touchscreen::setImage(uint8_t imageID)
{
    if (setImagePtr == NULL)
        return;

    (*setImagePtr)(imageID);
}

///
/// \brief Displays icon on display.
/// @param [in] x       X coordinate of icon.
/// @param [in] y       Y coordinate of icon.
/// @param [in] iconID  Number of icon to display.
///
void Touchscreen::setIcon(uint16_t x, uint16_t y, uint16_t iconID)
{
    if (setIconPtr == NULL)
        return;

    (*setIconPtr)(x, y, iconID);
}

Touchscreen touchscreen;