#ifndef Uart_h
#define Uart_h

#include <avr/io.h>

//minimal UART implementation

class UART    {

    public:
    UART();
    void begin(uint32_t baud_count, bool enableRX, bool enableTX);
    uint8_t available();
    int8_t read();
    void write(uint8_t);

};

extern UART uart;

#endif