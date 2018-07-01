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
#if !defined(BOARD_A_MEGA) && !defined(BOARD_A_UNO)
#include "../usb/Variables.h"
#endif


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

//some atmega models don't have number appended next to irq handler - assume 0
#ifdef USART_RX_vect
#define USART0_RX_vect USART_RX_vect
#endif

#ifdef USART_UDRE_vect
#define USART0_UDRE_vect USART_UDRE_vect
#endif

///
/// \brief ISR used to store incoming data from UART to buffer.
///
ISR(USART0_RX_vect)
{
    uint8_t data = UDR0;

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
        UCSR0B |= (1<<UDRIE0);
    }
}

///
/// \brief ISR used to write outgoing data in buffer to UART.
///
ISR(USART0_UDRE_vect)
{
    if (RingBuffer_IsEmpty(&txBuffer))
    {
        //buffer is empty, disable transmit interrupt
        UCSR0B &= ~(1<<UDRIE0);
    }
    else
    {
        uint8_t data = RingBuffer_Remove(&txBuffer);
        UDR0 = data;
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
    if (RingBuffer_IsEmpty(&txBuffer) && (UCSR0A & (1<<UDRE0)))
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            UDR0 = data;
        }

        #if !defined(BOARD_A_MEGA) && !defined(BOARD_A_UNO)
        MIDIsent = true;
        #endif

        return 1;
    }

    while (RingBuffer_IsFull(&txBuffer));
    RingBuffer_Insert(&txBuffer, data);
    UCSR0B |= (1<<UDRIE0);
    #if !defined(BOARD_A_MEGA) && !defined(BOARD_A_UNO)
    MIDIsent = true;
    #endif

    return 1;
}

#if !defined(BOARD_A_MEGA) && !defined(BOARD_A_UNO)
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
#endif

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
    #if !defined(BOARD_A_MEGA) && !defined(BOARD_A_UNO)
    MIDIreceived = true;
    #endif
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
            UCSR0A = 0;
            UCSR0B = 0;
            UCSR0C = 0;
            UBRR0 = 0;
        }

        if ((baud_count & 1) && baud_count <= 4096)
        {
            UCSR0A = (1<<U2X0); //double speed uart
            UBRR0 = baud_count - 1;
        }
        else
        {
            UCSR0A = 0;
            UBRR0 = (baud_count >> 1) - 1;
        }

        //8 bit, no parity, 1 stop bit
        UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);

        //enable receiver, transmitter and receive interrupt
        UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);

        RingBuffer_InitBuffer(&rxBuffer);
        RingBuffer_InitBuffer(&txBuffer);

        midi.handleUARTread(UARTread);
        midi.handleUARTwrite(UARTwrite);
    }
    #if !defined(BOARD_A_MEGA) && !defined(BOARD_A_UNO)
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
    #endif
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

#if !defined(BOARD_A_MEGA) && !defined(BOARD_A_UNO)
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
#endif