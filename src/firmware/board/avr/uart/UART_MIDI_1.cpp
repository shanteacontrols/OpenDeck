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
ISR(USART1_RX_vect)
{
    uint8_t data = UDR1;

    if (!RingBuffer_IsFull(&rxBuffer))
    {
        RingBuffer_Insert(&rxBuffer, data);
    }
}

///
/// \brief ISR signaling that transmission is done.
///
ISR(USART1_TX_vect)
{
    if (!RingBuffer_IsEmpty(&txBuffer))
    {
        uint8_t data = RingBuffer_Remove(&txBuffer);
        UDR1 = data;
    }
}

///
/// \brief Writes single byte to TX buffer.
/// @param [in] data    Byte value
/// \returns 0 on success, -1 otherwise.
///
int8_t UARTwrite(uint8_t data)
{
    if (!BIT_READ(UCSR1A, UDRE1))
    {
        //data transmission is already ongoing, store data in buffer
        if (RingBuffer_IsFull(&txBuffer))
            return -1;

        RingBuffer_Insert(&txBuffer, data);
    }
    else
    {
        UDR1 = data;
    }

    return 0;
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
