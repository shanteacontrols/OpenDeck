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