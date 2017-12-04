#include "Board.h"

MIDI midi;

///
/// \brief Buffer in which outgoing data is stored.
///
RingBuff_t  txBuffer;

///
/// \brief Buffer in which incoming data is stored.
///
RingBuff_t  rxBuffer;

///
/// \brief ISR used to store incoming data from UART to buffer.
///
ISR(USART1_RX_vect)
{
    uint8_t data = UDR1;

    if (!RingBuffer_IsFull(&rxBuffer))
    {
        RingBuffer_Insert(&rxBuffer, data);
    }
}

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

int8_t UARTwrite(uint8_t data)
{
    // If the buffer and the data register is empty, just write the byte
    // to the data register and be done. This shortcut helps
    // significantly improve the effective datarate at high (>
    // 500kbit/s) bitrates, where interrupt overhead becomes a slowdown.
    if (RingBuffer_IsEmpty(&txBuffer) && (UCSR1A & (1<<UDRE1)))
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            UDR1 = data;
        }

        return 1;
    }

    while (RingBuffer_IsFull(&txBuffer));
    RingBuffer_Insert(&txBuffer, data);
    UCSR1B |= (1<<UDRIE1);

    return 1;
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
    return data;
}

///
/// \brief Initializes UART peripheral used to send and receive MIDI data.
///
void Board::initUART_MIDI()
{
    int16_t baud_count = ((F_CPU / 8) + (31250 / 2)) / 31250;

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
    UCSR1B = (1<<RXEN1) | (1<<TXEN1) | (1<<RXCIE1);

    RingBuffer_InitBuffer(&rxBuffer);
    RingBuffer_InitBuffer(&txBuffer);

    midi.handleUARTread(UARTread);
    midi.handleUARTwrite(UARTwrite);
}