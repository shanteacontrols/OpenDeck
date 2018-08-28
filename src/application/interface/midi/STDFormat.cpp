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

#include "Handlers.h"
#include "board/common/uart/Variables.h"

///
/// \brief Internal function used as an callback for MIDI module for reading single byte of MIDI data from UART interface.
/// @param [in,out] data        Pointer to variable in which read data is being stored.
/// \returns False if no bytes are available, true otherwise.
///
bool uartReadMIDI(uint8_t &data)
{
    return board.uartRead(UART_MIDI_CHANNEL, data);
}

///
/// \brief Internal function used as an callback for MIDI module for writing single byte of MIDI data to UART interface.
/// \returns True on success, false otherwise.
///
bool uartWriteMIDI(uint8_t data)
{
    return board.uartWrite(UART_MIDI_CHANNEL, data);
}

void setupMIDIoverUART(uint8_t channel)
{
    board.initUART(UART_BAUDRATE_MIDI_STD, UART_MIDI_CHANNEL);
    midi.handleUARTread(uartReadMIDI);
    midi.handleUARTwrite(uartWriteMIDI);
}
