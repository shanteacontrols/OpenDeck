#include "../../Board.h"

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
ISR(USART0_RX_vect)
{
    uint8_t data = UDR0;

    if (!RingBuffer_IsFull(&rxBuffer))
    {
        RingBuffer_Insert(&rxBuffer, data);
    }
}

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

int8_t UARTwrite(uint8_t data)
{
    // If the buffer and the data register is empty, just write the byte
    // to the data register and be done. This shortcut helps
    // significantly improve the effective datarate at high (>
    // 500kbit/s) bitrates, where interrupt overhead becomes a slowdown.
    if (RingBuffer_IsEmpty(&txBuffer) && (UCSR0A & (1<<UDRE0)))
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            UDR0 = data;
        }

        return 1;
    }

    while (RingBuffer_IsFull(&txBuffer));
    RingBuffer_Insert(&txBuffer, data);
    UCSR0B |= (1<<UDRIE0);

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
    UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);

    RingBuffer_InitBuffer(&rxBuffer);
    RingBuffer_InitBuffer(&txBuffer);

    midi.handleUARTread(UARTread);
    midi.handleUARTwrite(UARTwrite);
}
