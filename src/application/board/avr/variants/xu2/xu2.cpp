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
#include "board/Board.h"
#include "board/common/uart/ODformat.h"

int main(void)
{
    Board::init();
    Board::initUART(UART_BAUDRATE_MIDI_OD, UART_USB_LINK_CHANNEL);

    sei();

    USBMIDIpacket_t USBMIDIpacket;
    Board::odPacketType_t packetType;

    while (1)
    {
        if (Board::usbReadMIDI(USBMIDIpacket))
            Board::uartWriteOD(UART_USB_LINK_CHANNEL, USBMIDIpacket, Board::odPacketType_t::packetMIDI);

        if (Board::uartReadOD(UART_USB_LINK_CHANNEL, USBMIDIpacket, packetType))
        {
            if (packetType != Board::odPacketType_t::packetIntCMD)
                Board::usbWriteMIDI(USBMIDIpacket);
        }

        static bool lastUSBstate = false;
        bool usbState = Board::isUSBconnected();

        if (usbState != lastUSBstate)
        {
            USBMIDIpacket.Event = usbState ? static_cast<uint8_t>(Board::odFormatCMD_t::cmdUsbStateConnected) : static_cast<uint8_t>(Board::odFormatCMD_t::cmdUsbStateNotConnected);
            USBMIDIpacket.Data1 = 0;
            USBMIDIpacket.Data2 = 0;
            USBMIDIpacket.Data3 = 0;

            Board::uartWriteOD(UART_USB_LINK_CHANNEL, USBMIDIpacket, Board::odPacketType_t::packetIntCMD);
            lastUSBstate = usbState;
        }
    }
}