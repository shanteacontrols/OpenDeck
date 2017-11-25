#ifdef BOARD_A_MEGA

#include "../Board.h"
#include "Common.h"

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

///
/// \brief ISR signaling that transmission is done.
///
ISR(USART0_TX_vect)
{
    if (!RingBuffer_IsEmpty(&txBuffer))
    {
        uint8_t data = RingBuffer_Remove(&txBuffer);
        UDR0 = data;
    }
}

///
/// \brief Writes single byte to TX buffer.
/// @param [in] data    Byte value
/// \returns 0 on success, -1 otherwise.
///
int8_t UARTwrite(uint8_t data)
{
    if (!BIT_READ(UCSR0A, UDRE0))
    {
        //data transmission is already ongoing, store data in buffer
        if (RingBuffer_IsFull(&txBuffer))
            return -1;

        RingBuffer_Insert(&txBuffer, data);
    }
    else
    {
        UDR0 = data;
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
        UCSR0A = (1<<U2X0); //double speed uart
        UBRR0 = baud_count - 1;
    }
    else
    {
        UCSR0A = 0;
        UBRR0 = (baud_count >> 1) - 1;
    }

    UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<TXCIE0) | (1<<RXCIE0);

    //8 bit, no parity, 1 stop bit
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);

    RingBuffer_InitBuffer(&rxBuffer);
    RingBuffer_InitBuffer(&txBuffer);

    midi.handleUARTread(UARTread);
    midi.handleUARTwrite(UARTwrite);
}

#endif