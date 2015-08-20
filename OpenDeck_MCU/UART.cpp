#include "UART.h"
#include <avr/interrupt.h>

//RX/TX buffer size in bytes
#define SERIAL_BUFFER_SIZE 64

static volatile uint8_t tx_buffer[SERIAL_BUFFER_SIZE] = { 255 };
static volatile uint8_t tx_buffer_counter = 0;

static volatile uint8_t rx_buffer[SERIAL_BUFFER_SIZE];
static volatile uint8_t rx_buffer_head = 0;
static volatile uint8_t rx_buffer_tail = 0;

#if defined (USART1_RX_vect)
    ISR(USART1_RX_vect) {

        uint8_t data, i;

        data = UDR1;
        i = rx_buffer_head + 1;
        if (i >= SERIAL_BUFFER_SIZE) i = 0;

        if (i != rx_buffer_tail) {
            rx_buffer[i] = data;
            rx_buffer_head = i;
        }

    }
#elif defined (USART_RX_vect)
    ISR(USART_RX_vect) {

        uint8_t data, i;

        data = UDR0;
        i = rx_buffer_head + 1;
        if (i >= SERIAL_BUFFER_SIZE) i = 0;

        if (i != rx_buffer_tail) {
            rx_buffer[i] = data;
            rx_buffer_head = i;
        }

    }
#endif

UART::UART()  {

    //default constructor

}


int8_t UART::read(void)   {

    uint8_t data, i;

    if (rx_buffer_head == rx_buffer_tail) return -1;
    i = rx_buffer_tail + 1;
    if (i >= SERIAL_BUFFER_SIZE) i = 0;
    data = rx_buffer[i];
    rx_buffer_tail = i;
    return data;

}

void UART::write(uint8_t data)   {

    //this will never happen, but whatever
    if (tx_buffer_counter >= SERIAL_BUFFER_SIZE) tx_buffer_counter = 0;

    tx_buffer[tx_buffer_counter] = data;
    tx_buffer_counter++;

}

void UART::begin(uint32_t baudRate)   {

    int16_t baud_count = ((F_CPU / 8) + (baudRate / 2)) / baudRate;

    if ((baud_count & 1) && baud_count <= 4096) {

        #if defined (UCSR1A)
            UCSR1A = (1<<U2X1);
        #elif defined (UCSR0A)
            UCSR0A = (1<<U2X0);
        #endif

        #if defined (UBRR1)
            UBRR1 = baud_count - 1;
        #elif defined (UBRR0)
            UBRR0 = baud_count - 1;
        #endif

    } else {

        #if defined (UCSR1A)
            UCSR1A = 0;
        #elif defined (UCSR0A)
            UCSR0A = 0;
        #endif

        #if defined (UBRR1)
            UBRR1 = (baud_count >> 1) - 1;
        #elif defined (UBRR0)
            UBRR0 = (baud_count >> 1) - 1;
        #endif

    }

    #if defined (UCSR1B) && defined (TXEN1)
        if (!(UCSR1B & (1<<TXEN1))) {

            rx_buffer_head = 0;
            rx_buffer_tail = 0;
            UCSR1C = (1<<UCSZ11) | (1<<UCSZ10);
            UCSR1B = (1<<RXEN1) | (1<<TXEN1) | (1<<RXCIE1);

        }
    #elif defined (UCSR0B) && defined (TXEN0)
        if (!(UCSR0B & (1<<TXEN0))) {

            rx_buffer_head = 0;
            rx_buffer_tail = 0;
            UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
            UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);

        }
    #endif

}

int16_t UART::availableRX()    {

    //return available number of bytes in incoming buffer

    uint8_t head, tail;

    head = rx_buffer_head;
    tail = rx_buffer_tail;
    if (head >= tail) return head - tail;
    return SERIAL_BUFFER_SIZE + head - tail;

}

uint8_t UART::availableTX()   {

    return tx_buffer_counter;

}

void UART::releaseTX()  {

    //apparently, UART doesn't like three timers
    //running so interrupt-based TX gets corrupted,
    //as well as led/button matrix

    //...or am i doing something wrong?

    //solution - release whole buffer when
    //data processing is finished

    //there is some data in buffer
    if (tx_buffer_counter) {

        for (int i=0; i<tx_buffer_counter; i++) {

            //wait until UDR is ready
            #ifdef UDR1
                while ((UCSR1A & (1 << UDRE1)) == 0) {};
                UDR1 = tx_buffer[i];
            #elif defined (UDR0)
                while ((UCSR0A & (1 << UDRE0)) == 0) {};
                UDR0 = tx_buffer[i];
            #endif

        }   tx_buffer_counter = 0;

    }

}


//initialize serial object
UART uart;