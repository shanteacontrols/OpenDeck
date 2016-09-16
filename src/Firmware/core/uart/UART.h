#ifndef Uart_h
#define Uart_h

#include <avr/io.h>

//minimal UART implementation

class UART    {

    public:
    UART();
    int8_t begin(uint32_t baud_count);
    bool available();
    int16_t read();
    int8_t write(uint8_t);

};

extern UART uart;

#endif