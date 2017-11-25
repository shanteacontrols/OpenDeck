#ifdef BOARD_OPEN_DECK

#include "Board.h"

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
/// \brief Empty ISR handler for UART transmission.
///
ISR(USART1_TX_vect)
{
    
}

///
/// \brief ISR used to send data from outgoing buffer to UART.
///
ISR(USART1_UDRE_vect)
{
    if (RingBuffer_IsEmpty(&txBuffer))
    {
        //buffer is empty, disable transmit interrupt
        UCSR1B = (1<<RXEN1) | (1<<TXCIE1) | (1<<TXEN1) | (1<<RXCIE1);
    }
    else
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
    if (RingBuffer_IsFull(&txBuffer))
    {
        return -1;
    }

    RingBuffer_Insert(&txBuffer, data);

    UCSR1B = (1<<RXEN1) | (1<<TXCIE1) | (1<<TXEN1) | (1<<RXCIE1) | (1<<UDRIE1);

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
    int16_t baud_count = ((F_CPU / 8) + (115200 / 2)) / 115200;

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

    UCSR1B = (1<<RXEN1) | (1<<TXCIE1) | (1<<TXEN1) | (1<<RXCIE1);

    //8 bit, no parity, 1 stop bit
    UCSR1C = (1<<UCSZ11) | (1<<UCSZ10);

    RingBuffer_InitBuffer(&rxBuffer);
    RingBuffer_InitBuffer(&txBuffer);

    midi.handleUARTread(UARTread);
    midi.handleUARTwrite(UARTwrite);
}

#endif