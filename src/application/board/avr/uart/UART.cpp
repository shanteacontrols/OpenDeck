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
#include "board/common/uart/Variables.h"
#include "Constants.h"

RingBuff_t   txBuffer[UART_INTERFACES];
RingBuff_t   rxBuffer[UART_INTERFACES];

///
/// \brief Flag determining whether or not UART loopback functionality is enabled.
/// When enabled, all incoming UART traffic is immediately passed on to UART TX.
///
static bool  loopbackEnabled[UART_INTERFACES];

///
/// \brief Flag signaling that the transmission is done.
///
static bool  txDone[UART_INTERFACES];


///
/// \brief ISR used to store incoming data from UART to buffer.
/// @{

ISR(USART_RX_vect_0)
{
    uint8_t data = UDR_0;

    if (!loopbackEnabled[0])
    {
        if (!RingBuffer_IsFull(&rxBuffer[0]))
        {
            RingBuffer_Insert(&rxBuffer[0], data);
            UARTreceived = true;
        }
    }
    else
    {
        if (!RingBuffer_IsFull(&txBuffer[0]))
        {
            RingBuffer_Insert(&txBuffer[0], data);
            UCSRB_0 |= (1<<UDRIE_0);
            UARTsent = true;
            UARTreceived = true;
        }
    }
}

#if UART_INTERFACES > 1
ISR(USART_RX_vect_1)
{
    uint8_t data = UDR_1;

    if (!loopbackEnabled[1])
    {
        if (!RingBuffer_IsFull(&rxBuffer[1]))
        {
            RingBuffer_Insert(&rxBuffer[1], data);
            UARTreceived = true;
        }
    }
    else
    {
        if (!RingBuffer_IsFull(&txBuffer[1]))
        {
            RingBuffer_Insert(&txBuffer[1], data);
            UCSRB_1 |= (1<<UDRIE_1);
            UARTsent = true;
            UARTreceived = true;
        }
    }
}
#endif

/// @}

///
/// \brief ISR used to write outgoing data in buffer to UART.
/// @{

ISR(USART_UDRE_vect_0)
{
    if (RingBuffer_IsEmpty(&txBuffer[0]))
    {
        //buffer is empty, disable transmit interrupt
        UCSRB_0 &= ~(1<<UDRIE_0);
    }
    else
    {
        uint8_t data = RingBuffer_Remove(&txBuffer[0]);
        UDR_0 = data;
        UARTsent = true;
    }
}

#if UART_INTERFACES > 1
ISR(USART_UDRE_vect_1)
{
    if (RingBuffer_IsEmpty(&txBuffer[1]))
    {
        //buffer is empty, disable transmit interrupt
        UCSRB_1 &= ~(1<<UDRIE_1);
    }
    else
    {
        uint8_t data = RingBuffer_Remove(&txBuffer[1]);
        UDR_1 = data;
        UARTsent = true;
    }
}
#endif

ISR(USART_TX_vect_0)
{
    txDone[0] = true;
}

#if UART_INTERFACES > 1
ISR(USART_TX_vect_1)
{
    txDone[1] = true;
}
#endif

/// @}

void Board::resetUART(uint8_t channel)
{
    if (channel >= UART_INTERFACES)
        return;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        switch(channel)
        {
            case 0:
            UCSRA_0 = 0;
            UCSRB_0 = 0;
            UCSRC_0 = 0;
            UBRR_0 = 0;
            break;

            #if UART_INTERFACES > 1
            case 1:
            UCSRA_1 = 0;
            UCSRB_1 = 0;
            UCSRC_1 = 0;
            UBRR_1 = 0;
            break;
            #endif

            default:
            break;
        }
    }

    RingBuffer_InitBuffer(&rxBuffer[channel]);
    RingBuffer_InitBuffer(&txBuffer[channel]);
}

void Board::initUART(uint32_t baudRate, uint8_t channel)
{
    if (channel >= UART_INTERFACES)
        return;

    resetUART(channel);

    int32_t baud_count = ((F_CPU / 8) + (baudRate / 2)) / baudRate;

    if ((baud_count & 1) && baud_count <= 4096)
    {
        switch(channel)
        {
            case 0:
            UCSRA_0 = (1<<U2X_0); //double speed uart
            UBRR_0 = baud_count - 1;
            break;

            #if UART_INTERFACES > 1
            case 1:
            UCSRA_1 = (1<<U2X_1); //double speed uart
            UBRR_1 = baud_count - 1;
            break;
            #endif

            default:
            break;
        }
    }
    else
    {
        switch(channel)
        {
            case 0:
            UCSRA_0 = 0;
            UBRR_0 = (baud_count >> 1) - 1;
            break;

            #if UART_INTERFACES > 1
            case 1:
            UCSRA_1 = 0;
            UBRR_1 = (baud_count >> 1) - 1;
            break;
            #endif

            default:
            break;
        }
    }

    //8 bit, no parity, 1 stop bit
    //enable receiver, transmitter and receive interrupt
    switch(channel)
    {
        case 0:
        UCSRC_0 = (1<<UCSZ1_0) | (1<<UCSZ0_0);
        UCSRB_0 = (1<<RXEN_0) | (1<<TXEN_0) | (1<<RXCIE_0) | (1<<TXCIE_0);
        break;

        #if UART_INTERFACES > 1
        case 1:
        UCSRC_1 = (1<<UCSZ1_1) | (1<<UCSZ0_1);
        UCSRB_1 = (1<<RXEN_1) | (1<<TXEN_1) | (1<<RXCIE_1) | (1<<TXCIE_1);
        break;
        #endif

        default:
        break;
    }
}

bool Board::uartRead(uint8_t channel, uint8_t &data)
{
    data = 0;

    if (channel >= UART_INTERFACES)
        return false;

    if (RingBuffer_IsEmpty(&rxBuffer[channel]))
        return false;

    data = RingBuffer_Remove(&rxBuffer[channel]);

    return true;
}

bool Board::uartWrite(uint8_t channel, uint8_t data)
{
    if (channel >= UART_INTERFACES)
        return false;

    //if both the outgoing buffer and the UART data register are empty
    //write the byte to the data register directly
    if (RingBuffer_IsEmpty(&txBuffer[channel]))
    {
        switch(channel)
        {
            case 0:
            if (UCSRA_0 & (1<<UDRE_0))
            {
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
                {
                    UDR_0 = data;
                    txDone[0] = false;
                }

                return true;
            }
            break;

            #if UART_INTERFACES > 1
            case 1:
            if (UCSRA_1 & (1<<UDRE_1))
            {
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
                {
                    UDR_1 = data;
                    txDone[1] = false;
                }

                return true;
            }
            break;
            #endif

            default:
            break;
        }
    }

    while (RingBuffer_IsFull(&txBuffer[channel]));
    RingBuffer_Insert(&txBuffer[channel], data);

    uartTransmitStart(channel);

    return true;
}

void Board::uartTransmitStart(uint8_t channel)
{
    if (channel >= UART_INTERFACES)
        return;

    txDone[channel] = false;

    switch(channel)
    {
        case 0:
        UCSRB_0 |= (1<<UDRIE_0);
        break;

        #if UART_INTERFACES > 1
        case 1:
        UCSRB_1 |= (1<<UDRIE_1);
        break;
        #endif

        default:
        break;
    }
}

void Board::setUARTloopbackState(uint8_t channel, bool state)
{
    if (channel >= UART_INTERFACES)
        return;

    loopbackEnabled[channel] = state;
}

bool Board::getUARTloopbackState(uint8_t channel)
{
    if (channel >= UART_INTERFACES)
        return false;

    return loopbackEnabled[channel];
}

bool Board::isUARTtxEmpty(uint8_t channel)
{
    if (channel >= UART_INTERFACES)
        return false;

    return txDone[channel];
}