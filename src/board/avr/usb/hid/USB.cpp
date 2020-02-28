/*

Copyright 2015-2020 Igor Petrovic

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

#include "board/common/usb/descriptors/Descriptors.h"
#include "board/Board.h"
#include "board/Internal.h"

///
/// \brief Event handler for the USB_ConfigurationChanged event.
/// This configures the device's endpoints ready to relay data to and from the attached USB host.
///
extern "C" void EVENT_USB_Device_ConfigurationChanged(void)
{
    //setup HID Report Endpoint
    Endpoint_ConfigureEndpoint(HID_IN_EPADDR, EP_TYPE_INTERRUPT, HID_IN_EPSIZE, 1);
}

/** This function is called by the library when in device mode, and must be overridden (see library "USB Descriptors"
 *  documentation) by the application code so that the address and size of a requested descriptor can be given
 *  to the USB library. When the device receives a Get Descriptor request on the control endpoint, this function
 *  is called so that the descriptor details can be passed back and the appropriate descriptor sent back to the
 *  USB host.
 */
extern "C" uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue, const uint16_t wIndex, const void** const DescriptorAddress)
{
    const uint8_t DescriptorType   = (wValue >> 8);
    const uint8_t DescriptorNumber = (wValue & 0xFF);

    const void* Address = NULL;
    uint16_t    Size    = NO_DESCRIPTOR;

    switch (DescriptorType)
    {
    case DTYPE_Device:
        Address = USBgetDeviceDescriptor(&Size);
        break;

    case DTYPE_Configuration:
        Address = USBgetCfgDescriptor(&Size);
        break;

    case HID_DTYPE_HID:
        Address = USBgetHIDdescriptor(&Size);
        break;

    case HID_DTYPE_Report:
        Address = USBgetHIDreport(&Size);
        break;

    case DTYPE_String:
        switch (DescriptorNumber)
        {
        case STRING_ID_Language:
            Address = USBgetLanguageString(&Size);
            break;
        case STRING_ID_Manufacturer:
            Address = USBgetManufacturerString(&Size);
            break;
        case STRING_ID_Product:
            Address = USBgetProductString(&Size);
            break;
        }
        break;
    }

    *DescriptorAddress = Address;
    return Size;
}

#ifdef FW_BOOT
///
/// \brief Event handler for the USB_ControlRequest event.
/// This is used to catch and process control requests sent to the device
/// from the USB host before passing along unhandled control requests to the
/// library for processing internally.
///
extern "C" void EVENT_USB_Device_ControlRequest(void)
{
    //ignore any requests that aren't directed to the HID interface
    if ((USB_ControlRequest.bmRequestType & (CONTROL_REQTYPE_TYPE | CONTROL_REQTYPE_RECIPIENT)) !=
        (REQTYPE_CLASS | REQREC_INTERFACE))
    {
        return;
    }

    //we are reading 2 bytes at the time
    uint32_t size = USB_ControlRequest.wLength / 2;

    //process HID specific control requests
    switch (USB_ControlRequest.bRequest)
    {
    case HID_REQ_SetReport:
        Endpoint_ClearSETUP();

        //wait until the command has been sent by the host
        while (!(Endpoint_IsOUTReceived()))
            ;

        for (uint32_t i = 0; i < size; i++)
        {
            //check if endpoint is empty - if so clear it and wait until ready for next packet
            if (!(Endpoint_BytesInEndpoint()))
            {
                Endpoint_ClearOUT();
                while (!(Endpoint_IsOUTReceived()))
                    ;
            }

            Board::bootloader::packetHandler(Endpoint_Read_16_LE());
        }

        Endpoint_ClearOUT();
        Endpoint_ClearStatusStage();
        break;
    }
}
#endif

namespace Board
{
    namespace detail
    {
        namespace setup
        {
            void usb()
            {
                USB_Init();
            }
        }    // namespace setup
    }        // namespace detail
}    // namespace Board