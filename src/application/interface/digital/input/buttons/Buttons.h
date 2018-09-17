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

#ifndef _INTERFACE_DIGITAL_INPUT_BUTTONS_
#define _INTERFACE_DIGITAL_INPUT_BUTTONS_

#include "DataTypes.h"

///
/// \brief Button handling.
/// \defgroup interfaceButtons Buttons
/// \ingroup interfaceDigitalIn
/// @{

class Buttons
{
    public:
    Buttons();

    static void init();
    static void update();
    static void processButton(uint8_t buttonID, bool state);
    static bool getButtonState(uint8_t buttonID);

    private:
    static void sendMessage(uint8_t buttonID, bool state);
    static void setButtonState(uint8_t buttonID, uint8_t state);
    static void setLatchingState(uint8_t buttonID, uint8_t state);
    static bool getLatchingState(uint8_t buttonID);
    static bool buttonDebounced(uint8_t buttonID, bool state);
};

///
/// \brief External definition of Buttons class instance.
///
extern Buttons buttons;

/// @}
#endif