/*
  Copyright 2020  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

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

#include <string.h>
#include <stdio.h>
#include "Descriptors.h"
#include "../Descriptors.h"
#include "board/common/comm/usb/descriptors/Constants.h"
#include "USBnames.h"
#include "core/src/general/Helpers.h"

/** Device descriptor structure. This descriptor, located in FLASH memory, describes the overall
 *  device characteristics, including the supported USB version, control endpoint size and the
 *  number of device configurations. The descriptor is read out by the USB host when the enumeration
 *  process begins.
 */
const USB_Descriptor_Device_t DeviceDescriptor = {
    .Header = {
        .Size = sizeof(USB_Descriptor_Device_t),
        .Type = DTYPE_Device,
    },

    .USBSpecification       = VERSION_BCD(2, 0, 0),
    .Class                  = CDC_CSCP_CDCClass,
    .SubClass               = CDC_CSCP_NoSpecificSubclass,
    .Protocol               = CDC_CSCP_NoSpecificProtocol,
    .Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,
    .VendorID               = USB_VENDOR_ID,
    .ProductID              = USB_PRODUCT_ID,
    .ReleaseNumber          = VERSION_BCD(1, 0, 0),
    .ManufacturerStrIndex   = STRING_ID_Manufacturer,
    .ProductStrIndex        = STRING_ID_Product,
    .SerialNumStrIndex      = STRING_ID_UID,
    .NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

/** Configuration descriptor structure. This descriptor, located in FLASH memory, describes the usage
 *  of the device in one of its supported configurations, including information about any device interfaces
 *  and endpoints. The descriptor is read out by the USB host during the enumeration process when selecting
 *  a configuration so that the host may correctly communicate with the USB device.
 */
const USB_Descriptor_Configuration_t ConfigurationDescriptor = {
    .Config = {
        .Header = {
            .Size = sizeof(USB_Descriptor_Configuration_Header_t),
            .Type = DTYPE_Configuration,
        },

        .TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
        .TotalInterfaces        = 2,
        .ConfigurationNumber    = 1,
        .ConfigurationStrIndex  = NO_DESCRIPTOR,
        .ConfigAttributes       = USB_CONF_DESC_ATTR_RESERVED,
        .MaxPowerConsumption    = USB_CONF_DESC_POWER_MA(100),
    },

    .CDC_CCI_Interface = {
        .Header = {
            .Size = sizeof(USB_Descriptor_Interface_t),
            .Type = DTYPE_Interface,
        },

        .InterfaceNumber   = INTERFACE_ID_CDC_CCI,
        .AlternateSetting  = 0,
        .TotalEndpoints    = 1,
        .Class             = CDC_CSCP_CDCClass,
        .SubClass          = CDC_CSCP_ACMSubclass,
        .Protocol          = CDC_CSCP_ATCommandProtocol,
        .InterfaceStrIndex = NO_DESCRIPTOR,
    },

    .CDC_Functional_Header = {
        .Header = {
            .Size = sizeof(USB_CDC_Descriptor_FunctionalHeader_t),
            .Type = CDC_DTYPE_CSInterface,
        },

        .Subtype          = CDC_DSUBTYPE_CSInterface_Header,
        .CDCSpecification = VERSION_BCD(1, 1, 0),
    },

    .CDC_Functional_ACM = {
        .Header = {
            .Size = sizeof(USB_CDC_Descriptor_FunctionalACM_t),
            .Type = CDC_DTYPE_CSInterface,
        },

        .Subtype      = CDC_DSUBTYPE_CSInterface_ACM,
        .Capabilities = 0x06,
    },

    .CDC_Functional_Union = {
        .Header = {
            .Size = sizeof(USB_CDC_Descriptor_FunctionalUnion_t),
            .Type = CDC_DTYPE_CSInterface,
        },

        .Subtype               = CDC_DSUBTYPE_CSInterface_Union,
        .MasterInterfaceNumber = INTERFACE_ID_CDC_CCI,
        .SlaveInterfaceNumber  = INTERFACE_ID_CDC_DCI,
    },

    .CDC_NotificationEndpoint = {
        .Header = {
            .Size = sizeof(USB_Descriptor_Endpoint_t),
            .Type = DTYPE_Endpoint,
        },

        .EndpointAddress   = CDC_NOTIFICATION_EPADDR,
        .Attributes        = (USB_EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
        .EndpointSize      = CDC_NOTIFICATION_EPSIZE,
        .PollingIntervalMS = 0xFF,
    },

    .CDC_DCI_Interface = {
        .Header = {
            .Size = sizeof(USB_Descriptor_Interface_t),
            .Type = DTYPE_Interface,
        },

        .InterfaceNumber   = INTERFACE_ID_CDC_DCI,
        .AlternateSetting  = 0,
        .TotalEndpoints    = 2,
        .Class             = CDC_CSCP_CDCDataClass,
        .SubClass          = CDC_CSCP_NoDataSubclass,
        .Protocol          = CDC_CSCP_NoDataProtocol,
        .InterfaceStrIndex = NO_DESCRIPTOR,
    },

    .CDC_DataOutEndpoint = {
        .Header = {
            .Size = sizeof(USB_Descriptor_Endpoint_t),
            .Type = DTYPE_Endpoint,
        },

        .EndpointAddress   = CDC_OUT_EPADDR,
        .Attributes        = (USB_EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
        .EndpointSize      = CDC_TXRX_EPSIZE,
        .PollingIntervalMS = CDC_POLLING_TIME,
    },

    .CDC_DataInEndpoint = {
        .Header = { .Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint },

        .EndpointAddress   = CDC_IN_EPADDR,
        .Attributes        = (USB_EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
        .EndpointSize      = CDC_TXRX_EPSIZE,
        .PollingIntervalMS = CDC_POLLING_TIME,
    }
};

/** Language descriptor structure. This descriptor, located in FLASH memory, is returned when the host requests
 *  the string descriptor with index 0 (the first index). It is actually an array of 16-bit integers, which indicate
 *  via the language ID table available at USB.org what languages the device supports for its string descriptors.
 */
const USB_Descriptor_String_t LanguageString = USB_STRING_DESCRIPTOR_ARRAY(LANGUAGE_ID_ENG);

/** Manufacturer descriptor string. This is a Unicode string containing the manufacturer's details in human readable
 *  form, and is read out upon request by the host when the appropriate string ID is requested, listed in the Device
 *  Descriptor.
 */
const USB_Descriptor_String_t ManufacturerString = USB_STRING_DESCRIPTOR(USB_MANUFACTURER);

/** Product descriptor string. This is a Unicode string containing the product's details in human readable form,
 *  and is read out upon request by the host when the appropriate string ID is requested, listed in the Device
 *  Descriptor.
 */
const USB_Descriptor_String_t ProductString = USB_STRING_DESCRIPTOR(USB_PRODUCT);

const USB_Descriptor_Configuration_t* USBgetCfgDescriptor(uint16_t* size)
{
    *size = sizeof(USB_Descriptor_Configuration_t);
    return &ConfigurationDescriptor;
}

const USB_Descriptor_Device_t* USBgetDeviceDescriptor(uint16_t* size)
{
    *size = sizeof(USB_Descriptor_Device_t);
    return &DeviceDescriptor;
}

const USB_Descriptor_String_t* USBgetLanguageString(uint16_t* size)
{
    *size = LanguageString.Header.Size;
    return &LanguageString;
}

const USB_Descriptor_String_t* USBgetManufacturerString(uint16_t* size)
{
    *size = ManufacturerString.Header.Size;
    return &ManufacturerString;
}

const USB_Descriptor_String_t* USBgetProductString(uint16_t* size)
{
    *size = ProductString.Header.Size;
    return &ProductString;
}

#ifdef UID_BITS
USB_Descriptor_UID_String_t SignatureDescriptorInternal;

const USB_Descriptor_UID_String_t* USBgetSerialIDString(uint16_t* size, uint8_t uid[])
{
    SignatureDescriptorInternal.Header.Type = DTYPE_String;
    SignatureDescriptorInternal.Header.Size = USB_STRING_LEN(UID_BITS / 4);

    uint8_t uidIndex = 0;

    for (int i = 0; i < UID_BITS / 4; i++)
    {
        uint8_t uidByte = uid[uidIndex];

        if (i & 0x01)
        {
            uidByte >>= 4;
            uidIndex++;
        }

        uidByte &= 0x0F;

        SignatureDescriptorInternal.UnicodeString[i] = (uidByte >= 10) ? (('A' - 10) + uidByte) : ('0' + uidByte);
    }

    *size = SignatureDescriptorInternal.Header.Size;
    return &SignatureDescriptorInternal;
}
#endif