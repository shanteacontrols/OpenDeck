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

#pragma once

#include "board/common/comm/usb/Arch.h"

#ifdef FW_CDC
#include "cdc/Descriptors.h"
#else
#include "midi/Descriptors.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    const USB_Descriptor_Configuration_t* USBgetCfgDescriptor(uint16_t* size);
    const USB_Descriptor_Device_t*        USBgetDeviceDescriptor(uint16_t* size);
    const USB_Descriptor_String_t*        USBgetLanguageString(uint16_t* size);
    const USB_Descriptor_String_t*        USBgetManufacturerString(uint16_t* size);
    const USB_Descriptor_String_t*        USBgetProductString(uint16_t* size);

#ifdef UID_BITS
    const USB_Descriptor_UID_String_t* USBgetSerialIDString(uint16_t* size, uint8_t uid[]);
#endif

#ifdef __cplusplus
}
#endif