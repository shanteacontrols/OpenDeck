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

#include "board/Board.h"
#include "board/common/uart/ODformat.h"
#include "board/common/uart/Variables.h"

bool Board::uartWriteMIDI_OD(uint8_t channel, USBMIDIpacket_t& USBMIDIpacket)
{
    if (channel >= UART_INTERFACES)
        return false;

    RingBuffer_Insert(&txBuffer[channel], OD_FORMAT_MIDI_DATA_START);
    RingBuffer_Insert(&txBuffer[channel], USBMIDIpacket.Event);
    RingBuffer_Insert(&txBuffer[channel], USBMIDIpacket.Data1);
    RingBuffer_Insert(&txBuffer[channel], USBMIDIpacket.Data2);
    RingBuffer_Insert(&txBuffer[channel], USBMIDIpacket.Data3);
    RingBuffer_Insert(&txBuffer[channel], USBMIDIpacket.Event ^ USBMIDIpacket.Data1 ^ USBMIDIpacket.Data2 ^ USBMIDIpacket.Data3);

    uartTransmitStart(channel);

    return true;
}

bool Board::uartReadMIDI_OD(uint8_t channel, USBMIDIpacket_t& USBMIDIpacket)
{
    if (channel >= UART_INTERFACES)
        return false;

    if (RingBuffer_GetCount(&rxBuffer[channel]) >= 6)
    {
        uint8_t data = 0;
        uartRead(channel, data);
        uint8_t dataXOR = 0;

        if (data == OD_FORMAT_MIDI_DATA_START)
        {
            //start of frame, read rest of the packet
            for (int i=0; i<5; i++)
            {
                Board::uartRead(channel, data);

                switch(i)
                {
                    case 0:
                    USBMIDIpacket.Event = data;
                    break;

                    case 1:
                    USBMIDIpacket.Data1 = data;
                    break;

                    case 2:
                    USBMIDIpacket.Data2 = data;
                    break;

                    case 3:
                    USBMIDIpacket.Data3 = data;
                    break;

                    case 4:
                    //xor byte, do nothing
                    break;
                }
            }

            //error check
            dataXOR = USBMIDIpacket.Event ^ USBMIDIpacket.Data1 ^ USBMIDIpacket.Data2 ^ USBMIDIpacket.Data3;

            return (dataXOR == data);
        }
        else if (data == OD_FORMAT_INT_DATA_START)
        {
            uint8_t cmd = 0;
            uartRead(channel, cmd);

            //ignore the rest of the buffer
            for (int i=0; i<4; i++)
                uartRead(channel, data);

            switch((odFormatCMD_t)cmd)
            {
                case cmdFwUpdated:
                ledFlashStartup(true);
                break;

                case cmdFwNotUpdated:
                ledFlashStartup(false);
                break;

                case cmdBtldrReboot:
                reboot(rebootBtldr);
                break;

                default:
                break;
            }
        }
    }

    return false;
}