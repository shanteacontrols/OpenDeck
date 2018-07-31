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

#pragma once

///
/// \brief Pointer to model-specific update() function.
///
extern bool         (*displayUpdatePtr)();

///
/// \brief Pointer to model-specific setImage() function.
///
extern void         (*setImagePtr)(uint8_t imageID);

///
/// \brief Pointer to model-specific setIcon() function.
///
extern void         (*setIconPtr)(uint16_t x, uint16_t y, uint16_t iconID);

///
/// \brief Pointer to external function used to read data from display.
///
extern int16_t      (*dataReadPtr)();

///
/// \brief Pointer to external function used to write data to display.
///
extern int8_t       (*dataWritePtr)(uint8_t data);

///
/// \brief Buffer holding incoming SDW data.
///
extern uint8_t      displayRxBuffer[TOUCHSCREEN_RX_BUFFER_SIZE];

///
/// \brief Current index in incoming data buffer.
///
extern uint8_t      bufferIndex_rx;

///
/// \brief Index of currently active (pressed or released) button on touchscreen.
/// Model-specific update() function should set the value of this variable if
/// there is an incoming message.
///
extern uint8_t      activeButtonID;

///
/// \brief State of currently active (pressed or released) button on touchscreen.
/// Model-specific update() function should set the value of this variable if
/// there is an incoming message.
///
extern bool         activeButtonState;