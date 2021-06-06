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

    .Audio_ControlInterface = {
        .Header = {
            .Size = sizeof(USB_Descriptor_Interface_t),
            .Type = DTYPE_Interface,
        },

        .InterfaceNumber   = INTERFACE_ID_AudioControl,
        .AlternateSetting  = 0,
        .TotalEndpoints    = 0,
        .Class             = AUDIO_CSCP_AudioClass,
        .SubClass          = AUDIO_CSCP_ControlSubclass,
        .Protocol          = AUDIO_CSCP_ControlProtocol,
        .InterfaceStrIndex = NO_DESCRIPTOR,
    },

    .Audio_ControlInterface_SPC = {
        .Header = {
            .Size = sizeof(USB_Audio_Descriptor_Interface_AC_t),
            .Type = AUDIO_DTYPE_CSInterface,
        },

        .Subtype         = AUDIO_DSUBTYPE_CSInterface_Header,
        .ACSpecification = VERSION_BCD(1, 0, 0),
        .TotalLength     = sizeof(USB_Audio_Descriptor_Interface_AC_t),
        .InCollection    = 1,
        .InterfaceNumber = INTERFACE_ID_AudioStream,
    },

    .Audio_StreamInterface = {
        .Header = {
            .Size = sizeof(USB_Descriptor_Interface_t),
            .Type = DTYPE_Interface,
        },

        .InterfaceNumber   = INTERFACE_ID_AudioStream,
        .AlternateSetting  = 0,
        .TotalEndpoints    = 2,
        .Class             = AUDIO_CSCP_AudioClass,
        .SubClass          = AUDIO_CSCP_MIDIStreamingSubclass,
        .Protocol          = AUDIO_CSCP_StreamingProtocol,
        .InterfaceStrIndex = NO_DESCRIPTOR,
    },

    .Audio_StreamInterface_SPC = {
        .Header = {
            .Size = sizeof(USB_MIDI_Descriptor_AudioInterface_AS_t),
            .Type = AUDIO_DTYPE_CSInterface,
        },

        .Subtype            = AUDIO_DSUBTYPE_CSInterface_General,
        .AudioSpecification = VERSION_BCD(1, 0, 0),
        .TotalLength        = (sizeof(USB_Descriptor_Configuration_t) - offsetof(USB_Descriptor_Configuration_t, Audio_StreamInterface_SPC)),
    },

    .MIDI_In_Jack_Emb = {
        .Header = {
            .Size = sizeof(USB_MIDI_Descriptor_InputJack_t),
            .Type = AUDIO_DTYPE_CSInterface,
        },

        .Subtype      = AUDIO_DSUBTYPE_CSInterface_InputTerminal,
        .JackType     = MIDI_JACKTYPE_Embedded,
        .JackID       = 0x01,
        .JackStrIndex = NO_DESCRIPTOR,
    },

    .MIDI_In_Jack_Ext = {
        .Header = {
            .Size = sizeof(USB_MIDI_Descriptor_InputJack_t),
            .Type = AUDIO_DTYPE_CSInterface,
        },

        .Subtype      = AUDIO_DSUBTYPE_CSInterface_InputTerminal,
        .JackType     = MIDI_JACKTYPE_External,
        .JackID       = 0x02,
        .JackStrIndex = NO_DESCRIPTOR,
    },

    .MIDI_Out_Jack_Emb = {
        .Header = {
            .Size = sizeof(USB_MIDI_Descriptor_OutputJack_t),
            .Type = AUDIO_DTYPE_CSInterface,
        },

        .Subtype      = AUDIO_DSUBTYPE_CSInterface_OutputTerminal,
        .JackType     = MIDI_JACKTYPE_Embedded,
        .JackID       = 0x03,
        .NumberOfPins = 1,
        .SourceJackID = { 0x02 },
        .SourcePinID  = { 0x01 },
        .JackStrIndex = NO_DESCRIPTOR,
    },

    .MIDI_Out_Jack_Ext = {
        .Header = {
            .Size = sizeof(USB_MIDI_Descriptor_OutputJack_t),
            .Type = AUDIO_DTYPE_CSInterface,
        },

        .Subtype      = AUDIO_DSUBTYPE_CSInterface_OutputTerminal,
        .JackType     = MIDI_JACKTYPE_External,
        .JackID       = 0x04,
        .NumberOfPins = 1,
        .SourceJackID = { 0x01 },
        .SourcePinID  = { 0x01 },
        .JackStrIndex = NO_DESCRIPTOR,
    },

    .MIDI_In_Jack_Endpoint = {
        .Endpoint = {
            .Header = {
                .Size = sizeof(USB_Audio_Descriptor_StreamEndpoint_Std_t),
                .Type = DTYPE_Endpoint,
            },

            .EndpointAddress   = MIDI_STREAM_OUT_EPADDR,
            .Attributes        = (USB_EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
            .EndpointSize      = MIDI_STREAM_EPSIZE,
            .PollingIntervalMS = 0x05,
        },

        .Refresh            = 0,
        .SyncEndpointNumber = 0,
    },

    .MIDI_In_Jack_Endpoint_SPC = {
        .Header = {
            .Size = sizeof(USB_MIDI_Descriptor_Jack_Endpoint_t),
            .Type = AUDIO_DTYPE_CSEndpoint,
        },

        .Subtype            = AUDIO_DSUBTYPE_CSEndpoint_General,
        .TotalEmbeddedJacks = 0x01,
        .AssociatedJackID   = { 0x01 },
    },

    .MIDI_Out_Jack_Endpoint = {
        .Endpoint = {
            .Header = {
                .Size = sizeof(USB_Audio_Descriptor_StreamEndpoint_Std_t),
                .Type = DTYPE_Endpoint,
            },

            .EndpointAddress   = MIDI_STREAM_IN_EPADDR,
            .Attributes        = (USB_EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
            .EndpointSize      = MIDI_STREAM_EPSIZE,
            .PollingIntervalMS = 0x05,
        },

        .Refresh            = 0,
        .SyncEndpointNumber = 0,
    },

    .MIDI_Out_Jack_Endpoint_SPC = {
        .Header = {
            .Size = sizeof(USB_MIDI_Descriptor_Jack_Endpoint_t),
            .Type = AUDIO_DTYPE_CSEndpoint,
        },

        .Subtype            = AUDIO_DSUBTYPE_CSEndpoint_General,
        .TotalEmbeddedJacks = 0x01,
        .AssociatedJackID   = { 0x03 },
    }
};

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
    .Class                  = USB_CSCP_NoDeviceClass,
    .SubClass               = USB_CSCP_NoDeviceSubclass,
    .Protocol               = USB_CSCP_NoDeviceProtocol,
    .Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,
    .VendorID               = USB_VENDOR_ID,
    .ProductID              = USB_PRODUCT_ID,
    .ReleaseNumber          = VERSION_BCD(0, 0, 1),
    .ManufacturerStrIndex   = STRING_ID_Manufacturer,
    .ProductStrIndex        = STRING_ID_Product,
    .SerialNumStrIndex      = STRING_ID_UID,
    .NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
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