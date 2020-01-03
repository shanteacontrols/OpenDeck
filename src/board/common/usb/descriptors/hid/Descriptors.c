/*
             LUFA Library
     Copyright (C) Dean Camera, 2015.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2015  Dean Camera (dean [at] fourwalledcubicle [dot] com)

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

#include <string.h>
#include <stdio.h>
#include "Descriptors.h"
#include "../Descriptors.h"
#include "UserConstants.h"
#include "core/src/general/Helpers.h"
#include "board/common/Common.h"

/** HID class report descriptor. This is a special descriptor constructed with values from the
 *  USBIF HID class specification to describe the reports and capabilities of the HID device. This
 *  descriptor is parsed by the host and its contents used to determine what data (and in what encoding)
 *  the device will send, and what it may be sent back from the host. Refer to the HID specification for
 *  more details on HID report descriptors.
 */
const USB_Descriptor_HIDReport_Datatype_t HIDReport[] =
{
    HID_RI_USAGE_PAGE(16, 0xFFDC), /* Vendor Page 0xDC */
    HID_RI_USAGE(8, 0xFB), /* Vendor Usage 0xFB */
    HID_RI_COLLECTION(8, 0x01), /* Vendor Usage 1 */
    HID_RI_USAGE(8, 0x02), /* Vendor Usage 2 */
    HID_RI_LOGICAL_MINIMUM(8, 0x00),
    HID_RI_LOGICAL_MAXIMUM(8, 0xFF),
    HID_RI_REPORT_SIZE(8, 0x08),
    HID_RI_REPORT_COUNT(16, (sizeof(uint16_t) + BTLDR_FLASH_PAGE_SIZE)),
    HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),
    HID_RI_END_COLLECTION(0),
};

/** Device descriptor structure. This descriptor, located in SRAM memory, describes the overall
 *  device characteristics, including the supported USB version, control endpoint size and the
 *  number of device configurations. The descriptor is read out by the USB host when the enumeration
 *  process begins.
 */
const USB_Descriptor_Device_t DeviceDescriptor =
{
    .Header                 = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},

    .USBSpecification       = VERSION_BCD(1,1,0),
    .Class                  = USB_CSCP_NoDeviceClass,
    .SubClass               = USB_CSCP_NoDeviceSubclass,
    .Protocol               = USB_CSCP_NoDeviceProtocol,

    .Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,

    .VendorID               = USB_VENDOR_ID,
    .ProductID              = USB_PRODUCT_ID,
    .ReleaseNumber          = VERSION_BCD(0,0,1),

    .ManufacturerStrIndex   = STRING_ID_Manufacturer,
    .ProductStrIndex        = STRING_ID_Product,
    .SerialNumStrIndex      = STRING_ID_UID,

    .NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

/** Configuration descriptor structure. This descriptor, located in SRAM memory, describes the usage
 *  of the device in one of its supported configurations, including information about any device interfaces
 *  and endpoints. The descriptor is read out by the USB host during the enumeration process when selecting
 *  a configuration so that the host may correctly communicate with the USB device.
 */
const USB_Descriptor_Configuration_t ConfigurationDescriptor =
{
    .Config =
        {
            .Header                 = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration},

            .TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
            .TotalInterfaces        = 1,

            .ConfigurationNumber    = 1,
            .ConfigurationStrIndex  = NO_DESCRIPTOR,

            .ConfigAttributes       = USB_CONFIG_ATTR_RESERVED,

            .MaxPowerConsumption    = USB_CONFIG_POWER_MA(100)
        },

    .HID_Interface =
        {
            .Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

            .InterfaceNumber        = INTERFACE_ID_GenericHID,
            .AlternateSetting       = 0x00,

            .TotalEndpoints         = 1,

            .Class                  = HID_CSCP_HIDClass,
            .SubClass               = HID_CSCP_NonBootSubclass,
            .Protocol               = HID_CSCP_NonBootProtocol,

            .InterfaceStrIndex      = NO_DESCRIPTOR
        },

    .HID_VendorHID =
        {
            .Header                 = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},

            .HIDSpec                = VERSION_BCD(1,1,1),
            .CountryCode            = 0x00,
            .TotalReportDescriptors = 1,
            .HIDReportType          = HID_DTYPE_Report,
            .HIDReportLength        = sizeof(HIDReport)
        },

    .HID_ReportINEndpoint =
        {
            .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

            .EndpointAddress        = HID_IN_EPADDR,
            .Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
            .EndpointSize           = HID_IN_EPSIZE,
            .PollingIntervalMS      = 0x05
        },
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

const USB_Descriptor_HIDReport_Datatype_t* USBgetHIDreport(uint16_t* size)
{
    *size = sizeof(HIDReport);
    return HIDReport;
}

const USB_HID_Descriptor_HID_t* USBgetHIDdescriptor(uint16_t* size)
{
    *size = sizeof(USB_HID_Descriptor_HID_t);
    return &ConfigurationDescriptor.HID_VendorHID;
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