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

///
/// \brief Default constructor.
///
Touchscreen::Touchscreen()
{
    sendReadCallback = NULL;
    sendWriteCallback = NULL;
}

///
/// \brief Continuously checks for data input from touch screen.
///
void Touchscreen::update()
{
    if (sendReadCallback == NULL)
        return;
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
/// \brief Configures callback to handle reading from display.
/// Handling function must return value -1 if no data is available.
///
void SDW::handleRead(int16_t(*fptr)())
{
    sendReadCallback = fptr;
}

///
/// \brief Configures callback to handle writing to display.
/// Handling function must return value -1 if writing has failed.
///
void SDW::handleWrite(int8_t(*fptr)(uint8_t data))
{
    sendWriteCallback = fptr;
}