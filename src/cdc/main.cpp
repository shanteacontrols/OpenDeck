/*

Copyright 2015-2021 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "board/Board.h"

#ifndef UART_CHANNEL_TOUCHSCREEN
#define UART_CHANNEL 0
#else
#define UART_CHANNEL UART_CHANNEL_TOUCHSCREEN
#endif

namespace
{
    char              _txBuffer[CDC_TXRX_EPSIZE];
    volatile uint32_t _baudrate;    //line handlers are called from interrupt

    void uartToUSB()
    {
        uint32_t size = 0;

        for (size = 0; size < CDC_TXRX_EPSIZE; size++)
        {
            uint8_t value;

            if (Board::UART::read(UART_CHANNEL, value))
            {
                _txBuffer[size] = value;
            }
            else
            {
                break;
            }
        }

        if (size)
        {
            while (!Board::USB::writeCDC(_txBuffer, size))
                ;

            Board::io::indicateTraffic(Board::io::dataSource_t::usb, Board::io::dataDirection_t::outgoing);
        }
    }

    void usbToUART()
    {
        char cdcByte;

        while (Board::USB::readCDC(cdcByte))
        {
            while (!Board::UART::write(UART_CHANNEL, cdcByte))
                ;

            Board::io::indicateTraffic(Board::io::dataSource_t::usb, Board::io::dataDirection_t::incoming);
        }
    }
}    // namespace

namespace Board
{
    namespace USB
    {
        void onCDCsetLineEncoding(uint32_t baudrate)
        {
            _baudrate = baudrate;
            Board::UART::init(UART_CHANNEL, _baudrate, true);
        }

        void onCDCgetLineEncoding(uint32_t& baudrate)
        {
            baudrate = _baudrate;
        }
    }    // namespace USB
}    // namespace Board

int main()
{
    Board::init();

    while (1)
    {
        if (_baudrate)
        {
            uartToUSB();
            usbToUART();
        }
    }
}