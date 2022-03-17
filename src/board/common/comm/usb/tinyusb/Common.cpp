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

#include "board/common/comm/usb/USB.h"
#include "board/Board.h"
#include "tusb.h"
#include <comm/usb/TinyUSB.h>

namespace
{
    volatile bool _usbConnected;
}    // namespace

extern "C" void TINY_USB_ISR_HANDLER(void)
{
    tud_int_handler(0);
}

extern "C" uint8_t const* tud_descriptor_device_cb(void)
{
    uint16_t                       size = 0;
    const USB_Descriptor_Device_t* desc = USBgetDeviceDescriptor(&size);
    return (uint8_t const*)desc;
}

extern "C" uint8_t const* tud_descriptor_configuration_cb(uint8_t index)
{
    uint16_t                              size = 0;
    const USB_Descriptor_Configuration_t* desc = USBgetCfgDescriptor(&size);
    return (uint8_t const*)desc;
}

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
extern "C" uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    uint16_t length;

    switch (index)
    {
    case STRING_ID_Language:
    {
        const USB_Descriptor_String_t* desc = USBgetLanguageString(&length);
        return (uint16_t*)desc;
    }
    break;

    case STRING_ID_Manufacturer:
    {
        const USB_Descriptor_String_t* desc = USBgetManufacturerString(&length);
        return (uint16_t*)desc;
    }
    break;

    case STRING_ID_Product:
    {
        const USB_Descriptor_String_t* desc = USBgetProductString(&length);
        return (uint16_t*)desc;
    }
    break;

    case STRING_ID_UID:
    {
        Board::uniqueID_t uid;
        Board::uniqueID(uid);

        const USB_Descriptor_UID_String_t* desc = USBgetSerialIDString(&length, &uid[0]);
        return (uint16_t*)desc;
    }
    break;

    default:
        return NULL;
    }
}

extern "C" void tud_mount_cb(void)
{
    _usbConnected = true;
}

extern "C" void tud_umount_cb(void)
{
    _usbConnected = false;
}

namespace Board::USB
{
    bool isUSBconnected()
    {
        return _usbConnected;
    }
}    // namespace Board::USB