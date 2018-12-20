/*

Copyright 2015-2018 Igor Petrovic

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