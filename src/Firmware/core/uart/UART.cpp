/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2017 Igor Petrovic

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

#include "UART.h"
#include "Config.h"
#include "ring_buffer/RingBuffer.h"

#if TX_ENABLE == 1 && RX_ENABLE == 1
RingBuff_t txBuffer, rxBuffer;
#elif TX_ENABLE == 1 && RX_ENABLE == 0
RingBuff_t txBuffer;
#elif TX_ENABLE == 0 && RX_ENABLE == 1
RingBuff_t rxBuffer;
#else
#error Please enable RX or TX
#endif

#if RX_ENABLE == 1
///
/// \brief ISR used to store incoming data from UART to buffer.
///
ISR(USART1_RX_vect)
{
    uint8_t data = UDR1;
    if (!RingBuffer_IsFull(&rxBuffer))
        RingBuffer_Insert(&rxBuffer, data);
}
#endif

#if TX_ENABLE == 1
///
/// \brief ISR used to send data from outgoing buffer to UART.
///
ISR(USART1_UDRE_vect)
{
    if (RingBuffer_IsEmpty(&txBuffer))
    {
        // buffer is empty, disable transmit interrupt
        #if RX_ENABLE == 0
        UCSR1B = (1<<TXCIE1) | (1<<TXEN1);
        #else
        UCSR1B = (1<<RXEN1) | (1<<TXCIE1) | (1<<TXEN1) | (1<<RXCIE1);
        #endif
    }
    else
    {
        uint8_t data = RingBuffer_Remove(&txBuffer);
        UDR1 = data;
    }
}

ISR(USART1_TX_vect) {}
#endif

///
/// \brief Default constructor (empty).
///
UART::UART()
{
    
}

///
/// \brief Initializes UART peripheral.
///
void UART::init(uint32_t baudRate)
{
    #if RX_ENABLE == 0 && TX_ENABLE == 0
    #error RX and TX are disabled, please enable at least one channel
    #endif

    int16_t baud_count = ((F_CPU / 8) + (baudRate / 2)) / baudRate;

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

    if (!(UCSR1B & (1<<TXEN1)))
    {
        //8 bit, no parity, 1 stop bit
        UCSR1C = (1<<UCSZ11) | (1<<UCSZ10);

        #if TX_ENABLE == 1 && RX_ENABLE == 1
        UCSR1B = (1<<RXEN1) | (1<<TXCIE1) | (1<<TXEN1) | (1<<RXCIE1);
        #elif RX_ENABLE == 1 && TX_ENABLE == 0
        UCSR1B = (1<<RXEN1) | (1<<RXCIE1);
        #elif RX_ENABLE == 0 && TX_ENABLE == 1
        UCSR1B = (1<<TXCIE1) | (1<<TXEN1);
        #endif
    }

    #if RX_ENABLE == 1 && TX_ENABLE == 1
    RingBuffer_InitBuffer(&txBuffer);
    RingBuffer_InitBuffer(&rxBuffer);
    #elif RX_ENABLE == 1 && TX_ENABLE == 0
    RingBuffer_InitBuffer(&rxBuffer);
    #elif RX_ENABLE == 0 && TX_ENABLE == 1
    RingBuffer_InitBuffer(&txBuffer);
    #endif
}

///
/// \brief Reads a byte from incoming UART buffer
/// \returns Single byte on success, -1 is buffer is empty.
///
int8_t UART::read(void)
{
    #if RX_ENABLE == 1
    if (RingBuffer_IsEmpty(&rxBuffer))
        return -1;
    uint8_t data = RingBuffer_Remove(&rxBuffer);
    return data;
    #else
    #error RX not enabled
    #endif
}

///
/// \brief Writes single byte to TX buffer.
/// @param [in] data    Byte value
/// \returns 0 on success, -1 otherwise.
///
int8_t UART::write(uint8_t data)
{
    #if TX_ENABLE == 1
    if (!(UCSR1B & (1<<TXEN1)))
        return -1;

    if (RingBuffer_IsFull(&txBuffer))
        return -1;

    RingBuffer_Insert(&txBuffer, data);

    #if RX_ENABLE == 0
    UCSR1B = (1<<TXCIE1) | (1<<TXEN1) | (1<<UDRIE1);
    #else
    UCSR1B = (1<<RXEN1) | (1<<TXCIE1) | (1<<TXEN1) | (1<<RXCIE1) | (1<<UDRIE1);
    #endif

    #else
    #error TX not enabled
    #endif

    return 0;
}

///
/// \brief Checks if any incoming UART data is available.
/// \returns True if any data is available, false otherwise.
///
bool UART::available()
{
    #if RX_ENABLE == 1
    return !RingBuffer_IsEmpty(&rxBuffer);
    #else
    #error RX not enabled
    #endif
}

UART uart;
