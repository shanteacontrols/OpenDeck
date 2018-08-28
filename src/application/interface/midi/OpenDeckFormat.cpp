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
/// \brief Holds currently active UART channel used for OpenDeck MIDI UART format.
///
uint8_t uartChannel_OD;


///
/// \brief Used to read MIDI data from UART formatted in OpenDeck format.
///
bool usbRead_OD(USBMIDIpacket_t& USBMIDIpacket)
{
    return uartReadMIDI_OD(uartChannel_OD);
}

///
/// \brief Used to write MIDI data to UART formatted in OpenDeck format.
///
bool usbWrite_OD(USBMIDIpacket_t& USBMIDIpacket)
{
    return uartWriteMIDI_OD(uartChannel_OD, USBMIDIpacket);
}

void setupMIDIoverUART_OD(uint8_t channel)
{
    uartChannel_OD = channel;

    #ifdef BOARD_OPEN_DECK
    if (board.isUSBconnected())
    {
        //master board
        //read usb midi data and forward it to uart in od format
        //write standard usb midi data
        //no need for uart handlers
        midi.handleUSBread(usbMIDItoUART_OD);
        midi.handleUSBwrite(board.usbWriteMIDI);
        midi.handleUARTread(NULL); //parsed internally
        midi.handleUARTwrite(NULL);
    }
    else
    {
        //slave
        midi.handleUSBread(usbRead_OD); //loopback used on inner slaves
        midi.handleUSBwrite(usbWrite_OD);
        //no need for uart handlers
        midi.handleUARTread(NULL);
        midi.handleUARTwrite(NULL);
    }
    #else
    #ifndef USB_SUPPORTED
    board.initUART(UART_BAUDRATE_MIDI_OD, uartChannel_OD);
    midi.handleUSBread(usbRead_OD);
    midi.handleUSBwrite(usbWrite_OD);
    midi.handleUARTread(NULL);
    midi.handleUARTwrite(NULL);
    #endif
    #endif
}

bool uartWriteMIDI_OD(uint8_t channel, USBMIDIpacket_t& USBMIDIpacket)
{
    if (channel >= UART_INTERFACES)
        return false;

    RingBuffer_Insert(&txBuffer[channel], OD_FORMAT_MIDI_DATA_START);
    RingBuffer_Insert(&txBuffer[channel], USBMIDIpacket.Event);
    RingBuffer_Insert(&txBuffer[channel], USBMIDIpacket.Data1);
    RingBuffer_Insert(&txBuffer[channel], USBMIDIpacket.Data2);
    RingBuffer_Insert(&txBuffer[channel], USBMIDIpacket.Data3);
    RingBuffer_Insert(&txBuffer[channel], USBMIDIpacket.Event ^ USBMIDIpacket.Data1 ^ USBMIDIpacket.Data2 ^ USBMIDIpacket.Data3);

    Board::uartTransmitStart(channel);

    return true;
}

bool uartReadMIDI_OD(uint8_t channel)
{
    if (channel >= UART_INTERFACES)
        return false;

    if (RingBuffer_GetCount(&rxBuffer[channel]) >= 6)
    {
        uint8_t data = 0;
        board.uartRead(channel, data);
        uint8_t dataXOR = 0;

        if (data == OD_FORMAT_MIDI_DATA_START)
        {
            //start of frame, read rest of the packet
            for (int i=0; i<5; i++)
            {
                board.uartRead(channel, data);

                switch(i)
                {
                    case 0:
                    usbMIDIpacket.Event = data;
                    break;

                    case 1:
                    usbMIDIpacket.Data1 = data;
                    break;

                    case 2:
                    usbMIDIpacket.Data2 = data;
                    break;

                    case 3:
                    usbMIDIpacket.Data3 = data;
                    break;

                    case 4:
                    //xor byte, do nothing
                    break;
                }
            }

            //error check
            dataXOR = usbMIDIpacket.Event ^ usbMIDIpacket.Data1 ^ usbMIDIpacket.Data2 ^ usbMIDIpacket.Data3;

            return (dataXOR == data);
        }
        else if (data == OD_FORMAT_INT_DATA_START)
        {
            uint8_t cmd = 0;
            board.uartRead(channel, cmd);

            //ignore the rest of the buffer
            for (int i=0; i<4; i++)
                board.uartRead(channel, data);

            switch((odFormatCMD_t)cmd)
            {
                case cmdFwUpdated:
                board.ledFlashStartup(true);
                break;

                case cmdFwNotUpdated:
                board.ledFlashStartup(false);
                break;

                case cmdBtldrReboot:
                board.reboot(rebootBtldr);
                break;

                default:
                break;
            }
        }
    }

    return false;
}

bool usbMIDItoUART_OD(USBMIDIpacket_t& USBMIDIpacket)
{
    if (Board::usbReadMIDI(usbMIDIpacket))
    {
        uartWriteMIDI_OD(uartChannel_OD, usbMIDIpacket);
        return true;
    }

    return false;
}
