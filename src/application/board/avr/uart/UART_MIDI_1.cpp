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
#include "board/common/indicators/Variables.h"
#include "../usb/Variables.h"


///
/// \brief Buffer in which outgoing data is stored.
///
static RingBuff_t  txBuffer;

///
/// \brief Buffer in which incoming data is stored.
///
static RingBuff_t  rxBuffer;

///
/// \brief Flag determining whether or not UART loopback functionality is enabled.
/// When enabled, all incoming UART traffic is immediately passed on to UART TX.
///
static bool        loopbackEnabled;

///
/// \brief ISR used to store incoming data from UART to buffer.
///
ISR(USART1_RX_vect)
{
    uint8_t data = UDR1;

    if (!loopbackEnabled)
    {
        if (!RingBuffer_IsFull(&rxBuffer))
        {
            RingBuffer_Insert(&rxBuffer, data);
        }
    }
    else
    {
        RingBuffer_Insert(&rxBuffer, data);
        UCSR1B |= (1<<UDRIE1);
    }
}

///
/// \brief ISR used to write outgoing data in buffer to UART.
///
ISR(USART1_UDRE_vect)
{
    if (RingBuffer_IsEmpty(&txBuffer))
    {
        //buffer is empty, disable transmit interrupt
        UCSR1B &= ~(1<<UDRIE1);
    }
    else
    {
        uint8_t data = RingBuffer_Remove(&txBuffer);
        UDR1 = data;
    }
}

///
/// \brief Writes a byte to outgoing UART buffer.
/// \returns Positive value on success. Since this function waits if
/// outgoig buffer is full, result will always be success (1).
///
int8_t UARTwrite(uint8_t data)
{
    //if both the outgoing buffer and the UART data register are empty
    //write the byte to the data register directly
    if (RingBuffer_IsEmpty(&txBuffer) && (UCSR1A & (1<<UDRE1)))
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            UDR1 = data;
        }

        MIDIsent = true;

        return 1;
    }

    while (RingBuffer_IsFull(&txBuffer));
    RingBuffer_Insert(&txBuffer, data);
    UCSR1B |= (1<<UDRIE1);
    MIDIsent = true;

    return 1;
}

///
/// \brief Writes to UART TX channel MIDI message formatted as USB packet.
/// \returns Positive value on success. Since this function waits if
/// outgoig buffer is full, result will always be success (1).
///
bool UARTwrite_odFormat(USBMIDIpacket_t& USBMIDIpacket)
{
    RingBuffer_Insert(&txBuffer, 0xF1);
    RingBuffer_Insert(&txBuffer, USBMIDIpacket.Event);
    RingBuffer_Insert(&txBuffer, USBMIDIpacket.Data1);
    RingBuffer_Insert(&txBuffer, USBMIDIpacket.Data2);
    RingBuffer_Insert(&txBuffer, USBMIDIpacket.Data3);
    RingBuffer_Insert(&txBuffer, USBMIDIpacket.Event ^ USBMIDIpacket.Data1 ^ USBMIDIpacket.Data2 ^ USBMIDIpacket.Data3);

    UCSR1B |= (1<<UDRIE1);
    MIDIsent = true;

    return true;
}

///
/// \brief Reads a byte from incoming UART buffer.
/// \returns Single byte on success, -1 is buffer is empty.
///
int16_t UARTread()
{
    if (RingBuffer_IsEmpty(&rxBuffer))
    {
        return -1;
    }

    uint8_t data = RingBuffer_Remove(&rxBuffer);
    MIDIreceived = true;
    return data;
}

void Board::initUART_MIDI(bool odFormat)
{
    if (!odFormat)
    {
        int32_t baud_count = ((F_CPU / 8) + (MIDI_BAUD_RATE / 2)) / MIDI_BAUD_RATE;

        //clear registers first
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            UCSR1A = 0;
            UCSR1B = 0;
            UCSR1C = 0;
            UBRR1 = 0;
        }

        if ((baud_count & 1) && baud_count <= 4096)
        {
            UCSR1A = (1<<U2X1); //double speed uart
            UBRR1 = baud_count - 1;
        }
        else
        {
            UCSR1A = 0;
            UBRR1 = (baud_count >> 1) - 1;
        }

        //8 bit, no parity, 1 stop bit
        UCSR1C = (1<<UCSZ11) | (1<<UCSZ10);

        //enable receiver, transmitter and receive interrupt
        UCSR1B = (1<<RXEN1) | (1<<TXEN1) | (1<<RXCIE1);

        RingBuffer_InitBuffer(&rxBuffer);
        RingBuffer_InitBuffer(&txBuffer);

        midi.handleUARTread(UARTread);
        midi.handleUARTwrite(UARTwrite);
    }
    else
    {
        if (!isUSBconnected())
        {
            //slave board - no usb read necessary
            midi.handleUSBread(NULL);
            //use usb write to send to uart
            midi.handleUSBwrite(UARTwrite_odFormat);
        }

        //no need for standard UART TX anymore
        midi.handleUARTwrite(NULL);
    }
}

void Board::setUARTloopbackState(bool state)
{
    loopbackEnabled = state;
}

bool Board::getUARTloopbackState()
{
    return loopbackEnabled;
}

bool Board::isRXempty()
{
    return RingBuffer_IsEmpty(&rxBuffer);
}

bool Board::isTXempty()
{
    return RingBuffer_IsEmpty(&txBuffer);
}

void Board::parseODuart()
{
    USBMIDIpacket_t USBMIDIpacket;
    int16_t data;

    if (UARTread() == 0xF1)
    {
        //start of frame, use recursive parsing
        for (int i=0; i<5; i++)
        {
            data = UARTread();

            if (data == -1)
                return;

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

        //everything fine so far
        MIDIreceived = true;

        uint8_t dataXOR = USBMIDIpacket.Event ^ USBMIDIpacket.Data1 ^ USBMIDIpacket.Data2 ^ USBMIDIpacket.Data3;

        if (dataXOR == data)
        {
            if (USB_DeviceState != DEVICE_STATE_Configured)
                return;

            Endpoint_SelectEndpoint(MIDI_Interface.Config.DataINEndpoint.Address);

            uint8_t ErrorCode;

            if ((ErrorCode = Endpoint_Write_Stream_LE(&USBMIDIpacket, sizeof(USBMIDIpacket_t), NULL)) != ENDPOINT_RWSTREAM_NoError)
                return;

            if (!(Endpoint_IsReadWriteAllowed()))
                Endpoint_ClearIN();

            MIDI_Device_Flush(&MIDI_Interface);

            MIDIsent = true;
        }
    }
}