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

///
/// \brief Event handler for the USB_ControlRequest event.
/// This is used to catch and process control requests sent to the device
/// from the USB host before passing along unhandled control requests to the
/// library for processing internally.
///
void EVENT_USB_Device_ControlRequest(void)
{
    uint16_t PageAddress;

    //ignore any requests that aren't directed to the HID interface
    if ((USB_ControlRequest.bmRequestType & (CONTROL_REQTYPE_TYPE | CONTROL_REQTYPE_RECIPIENT)) !=
        (REQTYPE_CLASS | REQREC_INTERFACE))
    {
        return;
    }

    //process HID specific control requests
    switch (USB_ControlRequest.bRequest)
    {
        case HID_REQ_SetReport:
        Endpoint_ClearSETUP();

        //wait until the command has been sent by the host
        while (!(Endpoint_IsOUTReceived()));
        //read in the write destination address
        PageAddress = Endpoint_Read_16_LE();

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
                //check if endpoint is empty - if so clear it and wait until ready for next packet
                if (!(Endpoint_BytesInEndpoint()))
                {
                    Endpoint_ClearOUT();
                    while (!(Endpoint_IsOUTReceived()));
                }

                //write the next data word to the FLASH page
                boot_page_fill(PageAddress + ((uint16_t)PageWord << 1), Endpoint_Read_16_LE());
            }

            //write the filled FLASH page to memory
            boot_page_write(PageAddress);
            boot_spm_busy_wait();

            //re-enable RWW section
            boot_rww_enable();
        }

        Endpoint_ClearOUT();
        Endpoint_ClearStatusStage();
        break;
    }
}