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
#include "Redef.h"

///
/// \brief Event handler for the USB_ControlRequest event.
/// This is used to catch and process control requests sent to the device
/// from the USB host before passing along unhandled control requests to the
/// library for processing internally.
///
void EVENT_USB_Device_ControlRequest(void)
{
    using namespace Board;
    using namespace Board::detail;

    //ignore any requests that aren't directed to the HID interface
    if ((USB_ControlRequest.bmRequestType & (CONTROL_REQTYPE_TYPE | CONTROL_REQTYPE_RECIPIENT)) !=
        (REQTYPE_CLASS | REQREC_INTERFACE))
    {
        return;
    }

    uint16_t PageAddress;

    //process HID specific control requests
    switch (USB_ControlRequest.bRequest)
    {
        case HID_REQ_SetReport:
        Endpoint_ClearSETUP();

        //wait until the command has been sent by the host
        while (!(Endpoint_IsOUTReceived()));
        //read in the write destination address
        PageAddress = Endpoint_Read_16_LE();

        //send magic sequence first
        for (int i=0; i<6; i++)
            Board::uartWrite(UART_USB_LINK_CHANNEL, bootloader::hidUploadStart[i]);

        Board::uartWrite(UART_USB_LINK_CHANNEL, LSB_WORD(PageAddress));
        Board::uartWrite(UART_USB_LINK_CHANNEL, MSB_WORD(PageAddress));

        if (PageAddress == COMMAND_STARTAPPLICATION)
        {
            while (!Board::isUARTtxEmpty(UART_USB_LINK_CHANNEL));
            bootloader::RunBootloader = false;
        }
        else if (PageAddress < BOOT_START_ADDR)
        {
            //write each of the FLASH page's bytes in sequence
            for (int PageWord=0; PageWord<SPM_PAGESIZE/2; PageWord++)
            {
                //check if endpoint is empty - if so clear it and wait until ready for next packet
                if (!(Endpoint_BytesInEndpoint()))
                {
                    Endpoint_ClearOUT();
                    while (!(Endpoint_IsOUTReceived()));
                }

                uint16_t dataWord = Endpoint_Read_16_LE();
                Board::uartWrite(UART_USB_LINK_CHANNEL, LSB_WORD(dataWord));
                Board::uartWrite(UART_USB_LINK_CHANNEL, MSB_WORD(dataWord));
            }
        }

        Endpoint_ClearOUT();
        Endpoint_ClearStatusStage();
        break;
    }
}