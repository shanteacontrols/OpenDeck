/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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