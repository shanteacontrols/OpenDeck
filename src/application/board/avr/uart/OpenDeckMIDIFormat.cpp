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
#include "board/common/uart/Variables.h"
#include "board/common/indicators/Variables.h"

namespace Board
{
    bool uartWriteOD(uint8_t channel, USBMIDIpacket_t& USBMIDIpacket, odPacketType_t packetType)
    {
        if (channel >= UART_INTERFACES)
            return false;

        uartWrite(channel, (uint8_t)packetType);
        uartWrite(channel, USBMIDIpacket.Event);
        uartWrite(channel, USBMIDIpacket.Data1);
        uartWrite(channel, USBMIDIpacket.Data2);
        uartWrite(channel, USBMIDIpacket.Data3);
        uartWrite(channel, USBMIDIpacket.Event ^ USBMIDIpacket.Data1 ^ USBMIDIpacket.Data2 ^ USBMIDIpacket.Data3);

        return true;
    }

    bool uartReadOD(uint8_t channel, USBMIDIpacket_t& USBMIDIpacket, odPacketType_t& packetType)
    {
        using namespace Board::detail;

        packetType = odPacketType_t::packetInvalid;

        if (channel >= UART_INTERFACES)
            return false;

        if (RingBuffer_GetCount(&rxBuffer[channel]) >= 6)
        {
            uint8_t data = 0;
            uartRead(channel, data);
            uint8_t dataXOR = 0;

            if ((data == OD_FORMAT_MIDI_DATA_START) || (data == OD_FORMAT_MIDI_DATA_MASTER_START))
            {
                packetType = (odPacketType_t)data;

                //start of frame, read rest of the packet
                for (int i=0; i<5; i++)
                {
                    uartRead(channel, data);

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
                    case odFormatCMD_t::cmdFwUpdated:
                    ledFlashStartup(true);
                    break;

                    case odFormatCMD_t::cmdFwNotUpdated:
                    ledFlashStartup(false);
                    break;

                    case odFormatCMD_t::cmdBtldrReboot:
                    reboot(rebootType_t::rebootBtldr);
                    break;

                    #ifndef USB_SUPPORTED
                    case odFormatCMD_t::cmdUsbStateConnected:
                    usbLinkState = true;
                    break;

                    case odFormatCMD_t::cmdUsbStateNotConnected:
                    usbLinkState = false;
                    break;
                    #endif

                    default:
                    break;
                }

                packetType = odPacketType_t::packetIntCMD;
                return true;
            }
        }

        return false;
    }
}