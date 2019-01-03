/*

Copyright 2015-2019 Igor Petrovic

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

#include "../BootloaderHID.h"
#include "board/Board.h"

void EVENT_UART_Device_ControlRequest()
{
    using namespace Board;
    using namespace Board::detail;

    uint16_t PageAddress;

    uint8_t lower;
    uint8_t upper;

    #if (FLASHEND > 0xFFFF)
    while (!Board::uartRead(UART_USB_LINK_CHANNEL, upper));
    while (!Board::uartRead(UART_USB_LINK_CHANNEL, lower));
    #else
    while (!Board::uartRead(UART_USB_LINK_CHANNEL, lower));
    while (!Board::uartRead(UART_USB_LINK_CHANNEL, upper));
    #endif

    PageAddress = GET_WORD(lower, upper);

    //check if the command is a program page command, or a start application command
    if (PageAddress == COMMAND_STARTAPPLICATION)
    {
        bootloader::RunBootloader = false;
    }
    else if (PageAddress < BOOT_START_ADDR)
    {
        //erase the given FLASH page, ready to be programmed
        boot_page_erase(PageAddress);
        boot_spm_busy_wait();

        //write each of the FLASH page's bytes in sequence
        for (int PageWord=0; PageWord<SPM_PAGESIZE/2; PageWord++)
        {
            while (!Board::uartRead(UART_USB_LINK_CHANNEL, lower));
            while (!Board::uartRead(UART_USB_LINK_CHANNEL, upper));
            uint16_t dataWord = GET_WORD(lower, upper);
            //write the next data word to the FLASH page
            uint16_t address = PageAddress + ((uint16_t)PageWord << 1);
            boot_page_fill(address, dataWord);
        }

        //write the filled FLASH page to memory
        boot_page_write(PageAddress);
        boot_spm_busy_wait();

        //re-enable RWW section
        boot_rww_enable();
    }
}