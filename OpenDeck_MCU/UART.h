#include <avr/io.h>

#ifndef Uart_h
#define Uart_h

#define SERIAL_MOD

//most of UART code copied from teensy core libs
//some modifications are made to transmission part

class UART    {

    public:
        UART();
        void begin(uint32_t baud_count);
        int16_t availableRX();
        uint8_t availableTX();
        int8_t read();
        void write(uint8_t);
        void releaseTX();

};

extern UART uart;

#endif