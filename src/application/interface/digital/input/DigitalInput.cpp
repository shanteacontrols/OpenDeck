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

#include "DigitalInput.h"
#include "board/Board.h"
#include "Variables.h"

///
/// \brief Used for buttonPCinc/buttonPCdec messages when each button press/encoder rotation sends incremented or decremented PC value.
/// 16 entries in array are used for 16 MIDI channels.
///
uint8_t     lastPCvalue[16];

///
/// \brief Default constructor.
///
DigitalInput::DigitalInput()
{
    
}

///
/// \brief Used to check if button and encoder data is available.
/// Data source (pins) for both buttons and encoders is same.
///
void DigitalInput::update()
{
    if (!board.digitalInputDataAvailable())
        return;

    buttons.update();
    encoders.update();
}

DigitalInput digitalInput;