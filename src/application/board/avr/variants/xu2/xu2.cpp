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

#include <avr/interrupt.h>
#include "Variables.h"
#include "board/Board.h"

int main(void)
{
    Board board;
    board.init();
    board.initUART(UART_BAUDRATE_MIDI_OD, UART_USB_LINK_CHANNEL);

    sei();

    USBMIDIpacket_t USBMIDIpacket;

    while (1)
    {
        if (board.usbReadMIDI(USBMIDIpacket))
            board.uartWriteMIDI_OD(UART_USB_LINK_CHANNEL, USBMIDIpacket);

        if (board.uartReadMIDI_OD(UART_USB_LINK_CHANNEL, USBMIDIpacket))
            board.usbWriteMIDI(USBMIDIpacket);
    }
}