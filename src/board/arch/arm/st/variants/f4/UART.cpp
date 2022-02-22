/*

Copyright 2015-2022 Igor Petrovic

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
#include "board/Internal.h"
#include <MCU.h>

#if defined(FW_APP)
// not needed in bootloader
#ifdef UART_SUPPORTED
extern "C" void USART1_IRQHandler(void)
{
    Board::detail::isrHandling::uart(0);
}

extern "C" void USART2_IRQHandler(void)
{
    Board::detail::isrHandling::uart(1);
}

extern "C" void USART3_IRQHandler(void)
{
    Board::detail::isrHandling::uart(2);
}

extern "C" void UART4_IRQHandler(void)
{
    Board::detail::isrHandling::uart(3);
}

extern "C" void UART5_IRQHandler(void)
{
    Board::detail::isrHandling::uart(4);
}

extern "C" void USART6_IRQHandler(void)
{
    Board::detail::isrHandling::uart(5);
}
#endif
#endif