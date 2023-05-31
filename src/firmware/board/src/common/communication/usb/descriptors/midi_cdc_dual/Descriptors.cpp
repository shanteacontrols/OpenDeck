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

#ifdef PROJECT_TARGET_SUPPORT_USB
#ifdef FW_APP

#include <string.h>
#include <stdio.h>
#include "board/src/Internal.h"
#include "Descriptors.h"
#include "core/MCU.h"
#include "board/src/common/communication/usb/Constants.h"
#include "board/src/common/communication/usb/descriptors/common/Common.h"
#include "core/util/Util.h"
#include <Target.h>

namespace
{
    constexpr uint8_t USB_CDC_POLLING_TIME = 5;

    const usbDescriptorConfiguration_t PROGMEM _configurationDescriptor = {
        .config = {
            .header = {
                .size = sizeof(core::mcu::usb::descriptorConfigurationHeader_t),
                .type = core::mcu::usb::DESC_TYPE_CONFIGURATION,
            },

            .totalConfigurationSize = sizeof(usbDescriptorConfiguration_t),
            .totalInterfaces        = 4,
            .configurationNumber    = 1,
            .configurationStrIndex  = core::mcu::usb::NO_DESCRIPTOR,
            .configAttributes       = core::mcu::usb::CONF_DESC_ATTR_RESERVED,
            .maxPowerConsumption    = core::mcu::usb::CONF_DESC_POWER_MA(100),
        },

        .cdcIad = {
            .header = {
                .size = sizeof(core::mcu::usb::descriptorInterfaceAssociation_t),
                .type = core::mcu::usb::DESC_TYPE_INTERFACE_ASSOCIATION,
            },

            .firstInterfaceIndex = USB_INTERFACE_ID_CDC_CCI,
            .totalInterfaces     = 2,

            .classId    = core::mcu::usb::CDC_CSCP_CDC_CLASS,
            .subClassId = core::mcu::usb::CDC_CSCP_ACM_SUBCLASS,
            .protocol   = core::mcu::usb::CDC_CSCP_AT_COMMAND_PROTOCOL,

            .iadStrIndex = core::mcu::usb::NO_DESCRIPTOR,
        },

        .cdcCciInterface = {
            .header = {
                .size = sizeof(core::mcu::usb::descriptorInterface_t),
                .type = core::mcu::usb::DESC_TYPE_INTERFACE,
            },

            .interfaceNumber   = USB_INTERFACE_ID_CDC_CCI,
            .alternateSetting  = 0,
            .totalEndpoints    = 1,
            .classId           = core::mcu::usb::CDC_CSCP_CDC_CLASS,
            .subClassId        = core::mcu::usb::CDC_CSCP_ACM_SUBCLASS,
            .protocol          = core::mcu::usb::CDC_CSCP_AT_COMMAND_PROTOCOL,
            .interfaceStrIndex = core::mcu::usb::NO_DESCRIPTOR,
        },

        .cdcFunctionalHeader = {
            .header = {
                .size = sizeof(core::mcu::usb::cdcDescriptorFunctionalHeader_t),
                .type = core::mcu::usb::CDC_DTYPE_CS_INTERFACE,
            },

            .subType          = core::mcu::usb::CDC_DSUBTYPE_CS_INTERFACE_HEADER,
            .cdcSpecification = core::mcu::usb::VERSION_BCD(1, 1, 0),
        },

        .cdcFunctionalAcm = {
            .header = {
                .size = sizeof(core::mcu::usb::cdcDescriptorFunctionalACM_t),
                .type = core::mcu::usb::CDC_DTYPE_CS_INTERFACE,
            },

            .subType      = core::mcu::usb::CDC_DSUBTYPE_CS_INTERFACE_ACM,
            .capabilities = 0x06,
        },

        .cdcFunctionalUnion = {
            .header = {
                .size = sizeof(core::mcu::usb::cdcDescriptorFunctionalUnion_t),
                .type = core::mcu::usb::CDC_DTYPE_CS_INTERFACE,
            },

            .subType               = core::mcu::usb::CDC_DSUBTYPE_CS_INTERFACE_UNION,
            .masterInterfaceNumber = USB_INTERFACE_ID_CDC_CCI,
            .slaveInterfaceNumber  = USB_INTERFACE_ID_CDC_DCI,
        },

        .cdcNotificationEndpoint = {
            .header = {
                .size = sizeof(core::mcu::usb::descriptorEndpoint_t),
                .type = core::mcu::usb::DESC_TYPE_ENDPOINT,
            },

            .endpointAddress   = PROJECT_MCU_USB_ENDPOINT_MIDI_CDC_DUAL_ADDR_CDC_NOTIFICATION,
            .attributes        = (core::mcu::usb::ENDPOINT_TYPE_INTERRUPT | core::mcu::usb::ENDPOINT_ATTR_NO_SYNC | core::mcu::usb::ENDPOINT_USAGE_DATA),
            .endpointSize      = PROJECT_MCU_USB_ENDPOINT_SIZE_CDC_NOTIFICATION,
            .pollingIntervalMs = 0xFF,
        },

        .cdcDciInterface = {
            .header = {
                .size = sizeof(core::mcu::usb::descriptorInterface_t),
                .type = core::mcu::usb::DESC_TYPE_INTERFACE,
            },

            .interfaceNumber   = USB_INTERFACE_ID_CDC_DCI,
            .alternateSetting  = 0,
            .totalEndpoints    = 2,
            .classId           = core::mcu::usb::CDC_CSCP_CDC_DATA_CLASS,
            .subClassId        = core::mcu::usb::CDC_CSCP_NO_DATA_SUBCLASS,
            .protocol          = core::mcu::usb::CDC_CSCP_NO_DATA_PROTOCOL,
            .interfaceStrIndex = core::mcu::usb::NO_DESCRIPTOR,
        },

        .cdcDataOutEndpoint = {
            .header = {
                .size = sizeof(core::mcu::usb::descriptorEndpoint_t),
                .type = core::mcu::usb::DESC_TYPE_ENDPOINT,
            },

            .endpointAddress   = PROJECT_MCU_USB_ENDPOINT_MIDI_CDC_DUAL_ADDR_CDC_OUT,
            .attributes        = (core::mcu::usb::ENDPOINT_TYPE_BULK | core::mcu::usb::ENDPOINT_ATTR_NO_SYNC | core::mcu::usb::ENDPOINT_USAGE_DATA),
            .endpointSize      = PROJECT_MCU_USB_ENDPOINT_SIZE_CDC_IN_OUT,
            .pollingIntervalMs = USB_CDC_POLLING_TIME,
        },

        .cdcDataInEndpoint = {
            .header = {
                .size = sizeof(core::mcu::usb::descriptorEndpoint_t),
                .type = core::mcu::usb::DESC_TYPE_ENDPOINT,
            },

            .endpointAddress   = PROJECT_MCU_USB_ENDPOINT_MIDI_CDC_DUAL_ADDR_CDC_IN,
            .attributes        = (core::mcu::usb::ENDPOINT_TYPE_BULK | core::mcu::usb::ENDPOINT_ATTR_NO_SYNC | core::mcu::usb::ENDPOINT_USAGE_DATA),
            .endpointSize      = PROJECT_MCU_USB_ENDPOINT_SIZE_CDC_IN_OUT,
            .pollingIntervalMs = USB_CDC_POLLING_TIME,
        },

        .audioControlInterface = {
            .header = {
                .size = sizeof(core::mcu::usb::descriptorInterface_t),
                .type = core::mcu::usb::DESC_TYPE_INTERFACE,
            },

            .interfaceNumber   = USB_INTERFACE_ID_AUDIO_CONTROL,
            .alternateSetting  = 0,
            .totalEndpoints    = 0,
            .classId           = core::mcu::usb::AUDIO_CSCP_AUDIO_CLASS,
            .subClassId        = core::mcu::usb::AUDIO_CSCP_CONTROL_SUBCLASS,
            .protocol          = core::mcu::usb::AUDIO_CSCP_CONTROL_PROTOCOL,
            .interfaceStrIndex = core::mcu::usb::NO_DESCRIPTOR,
        },

        .audioControlInterfaceSpc = {
            .header = {
                .size = sizeof(core::mcu::usb::audioDescriptorInterfaceAC_t),
                .type = core::mcu::usb::AUDIO_DTYPE_CS_INTERFACE,
            },

            .subType         = core::mcu::usb::AUDIO_DSUBTYPE_CS_INTERFACE_HEADER,
            .acSpecification = core::mcu::usb::VERSION_BCD(1, 0, 0),
            .totalLength     = sizeof(core::mcu::usb::audioDescriptorInterfaceAC_t),
            .inCollection    = 1,
            .interfaceNumber = USB_INTERFACE_ID_AUDIO_STREAM,
        },

        .audioStreamInterface = {
            .header = {
                .size = sizeof(core::mcu::usb::descriptorInterface_t),
                .type = core::mcu::usb::DESC_TYPE_INTERFACE,
            },

            .interfaceNumber   = USB_INTERFACE_ID_AUDIO_STREAM,
            .alternateSetting  = 0,
            .totalEndpoints    = 2,
            .classId           = core::mcu::usb::AUDIO_CSCP_AUDIO_CLASS,
            .subClassId        = core::mcu::usb::AUDIO_CSCP_MIDI_STREAMING_SUBCLASS,
            .protocol          = core::mcu::usb::AUDIO_CSCP_STREAMING_PROTOCOL,
            .interfaceStrIndex = core::mcu::usb::NO_DESCRIPTOR,
        },

        .audioStreamInterfaceSpc = {
            .header = {
                .size = sizeof(core::mcu::usb::midiDescriptorAudioInterfaceAS_t),
                .type = core::mcu::usb::AUDIO_DTYPE_CS_INTERFACE,
            },

            .subType            = core::mcu::usb::AUDIO_DSUBTYPE_CS_INTERFACE_GENERAL,
            .audioSpecification = core::mcu::usb::VERSION_BCD(1, 0, 0),
            .totalLength        = (sizeof(usbDescriptorConfiguration_t) - offsetof(usbDescriptorConfiguration_t, audioStreamInterfaceSpc)),
        },

        .midiInJackEmb = {
            .header = {
                .size = sizeof(core::mcu::usb::midiDescriptorInputJack_t),
                .type = core::mcu::usb::AUDIO_DTYPE_CS_INTERFACE,
            },

            .subType      = core::mcu::usb::AUDIO_DSUBTYPE_CS_INTERFACE_INPUT_TERMINAL,
            .jackType     = core::mcu::usb::MIDI_JACKTYPE_EMBEDDED,
            .jackId       = 0x01,
            .jackStrIndex = core::mcu::usb::NO_DESCRIPTOR,
        },

        .midiInJackExt = {
            .header = {
                .size = sizeof(core::mcu::usb::midiDescriptorInputJack_t),
                .type = core::mcu::usb::AUDIO_DTYPE_CS_INTERFACE,
            },

            .subType      = core::mcu::usb::AUDIO_DSUBTYPE_CS_INTERFACE_INPUT_TERMINAL,
            .jackType     = core::mcu::usb::MIDI_JACKTYPE_EXTERNAL,
            .jackId       = 0x02,
            .jackStrIndex = core::mcu::usb::NO_DESCRIPTOR,
        },

        .midiOutJackEmb = {
            .header = {
                .size = sizeof(core::mcu::usb::midiDescriptorOutputJack_t),
                .type = core::mcu::usb::AUDIO_DTYPE_CS_INTERFACE,
            },

            .subType      = core::mcu::usb::AUDIO_DSUBTYPE_CS_INTERFACE_OUTPUT_TERMINAL,
            .jackType     = core::mcu::usb::MIDI_JACKTYPE_EMBEDDED,
            .jackId       = 0x03,
            .numberOfPins = 1,
            .sourceJackId = {
                0x02,
            },
            .sourcePinId = {
                0x01,
            },
            .jackStrIndex = core::mcu::usb::NO_DESCRIPTOR,
        },

        .midiOutJackExt = {
            .header = {
                .size = sizeof(core::mcu::usb::midiDescriptorOutputJack_t),
                .type = core::mcu::usb::AUDIO_DTYPE_CS_INTERFACE,
            },

            .subType      = core::mcu::usb::AUDIO_DSUBTYPE_CS_INTERFACE_OUTPUT_TERMINAL,
            .jackType     = core::mcu::usb::MIDI_JACKTYPE_EXTERNAL,
            .jackId       = 0x04,
            .numberOfPins = 1,
            .sourceJackId = {
                0x01,
            },
            .sourcePinId = {
                0x01,
            },
            .jackStrIndex = core::mcu::usb::NO_DESCRIPTOR,
        },

        .midiInJackEndPoint = {
            .endpoint = {
                .header = {
                    .size = sizeof(core::mcu::usb::audioDescriptorStreamEndpointStd_t),
                    .type = core::mcu::usb::DESC_TYPE_ENDPOINT,
                },

                .endpointAddress   = PROJECT_MCU_USB_ENDPOINT_MIDI_CDC_DUAL_ADDR_MIDI_OUT,
                .attributes        = (core::mcu::usb::ENDPOINT_TYPE_BULK | core::mcu::usb::ENDPOINT_ATTR_NO_SYNC | core::mcu::usb::ENDPOINT_USAGE_DATA),
                .endpointSize      = PROJECT_MCU_USB_ENDPOINT_SIZE_MIDI_IN_OUT,
                .pollingIntervalMs = 0x05,
            },

            .refresh            = 0,
            .syncEndpointNumber = 0,
        },

        .midiInJackEndPointSpc = {
            .header = {
                .size = sizeof(core::mcu::usb::midiDescriptorJackEndpoint_t),
                .type = core::mcu::usb::AUDIO_DTYPE_CS_ENDPOINT,
            },

            .subType            = core::mcu::usb::AUDIO_DSUBTYPE_CS_ENDPOINT_GENERAL,
            .totalEmbeddedJacks = 0x01,
            .associatedJackId   = {
                  0x01,
            },
        },

        .midiOutJackEndpoint = {
            .endpoint = {
                .header = {
                    .size = sizeof(core::mcu::usb::audioDescriptorStreamEndpointStd_t),
                    .type = core::mcu::usb::DESC_TYPE_ENDPOINT,
                },

                .endpointAddress   = PROJECT_MCU_USB_ENDPOINT_MIDI_CDC_DUAL_ADDR_MIDI_IN,
                .attributes        = (core::mcu::usb::ENDPOINT_TYPE_BULK | core::mcu::usb::ENDPOINT_ATTR_NO_SYNC | core::mcu::usb::ENDPOINT_USAGE_DATA),
                .endpointSize      = PROJECT_MCU_USB_ENDPOINT_SIZE_MIDI_IN_OUT,
                .pollingIntervalMs = 0x05,
            },

            .refresh            = 0,
            .syncEndpointNumber = 0,
        },

        .midiOutJackEndpointSpc = {
            .header = {
                .size = sizeof(core::mcu::usb::midiDescriptorJackEndpoint_t),
                .type = core::mcu::usb::AUDIO_DTYPE_CS_ENDPOINT,
            },

            .subType            = core::mcu::usb::AUDIO_DSUBTYPE_CS_ENDPOINT_GENERAL,
            .totalEmbeddedJacks = 0x01,
            .associatedJackId   = {
                  0x03,
            },
        }
    };

    const core::mcu::usb::descriptorDevice_t PROGMEM _deviceDescriptor = {
        .header = {
            .size = sizeof(core::mcu::usb::descriptorDevice_t),
            .type = core::mcu::usb::DESC_TYPE_DEVICE,
        },

        .usbSpecification       = core::mcu::usb::VERSION_BCD(2, 0, 0),
        .classId                = core::mcu::usb::CSCP_IAD_DEVICE_CLASS,
        .subClassId             = core::mcu::usb::CSCP_IAD_DEVICE_SUBCLASS,
        .protocol               = core::mcu::usb::CSCP_IAD_DEVICE_PROTOCOL,
        .endpoint0Size          = PROJECT_MCU_USB_ENDPOINT_SIZE_CONTROL,
        .vendorId               = USB_VENDOR_ID,
        .productId              = USB_PRODUCT_ID,
        .releaseNumber          = core::mcu::usb::VERSION_BCD(0, 0, 1),
        .manufacturerStrIndex   = USB_STRING_ID_MANUFACTURER,
        .productStrIndex        = USB_STRING_ID_PRODUCT,
        .serialNumStrIndex      = USB_STRING_ID_UID,
        .numberOfConfigurations = 1
    };

    const core::mcu::usb::descriptorString_t PROGMEM _languageString     = CORE_MCU_USB_STRING_DESCRIPTOR_ARRAY(core::mcu::usb::LANGUAGE_ID_ENG);
    const core::mcu::usb::descriptorString_t PROGMEM _manufacturerString = CORE_MCU_USB_STRING_DESCRIPTOR(USB_MANUFACTURER_NAME);
    const core::mcu::usb::descriptorString_t PROGMEM _productString      = CORE_MCU_USB_STRING_DESCRIPTOR(PROJECT_TARGET_USB_NAME);
}    // namespace

namespace board::detail::usb
{
    const void* cfgDescriptor(uint16_t* size)
    {
        *size = sizeof(usbDescriptorConfiguration_t);
        return &_configurationDescriptor;
    }

    const void* deviceDescriptor(uint16_t* size)
    {
        *size = sizeof(core::mcu::usb::descriptorDevice_t);
        return &_deviceDescriptor;
    }

    const void* languageString(uint16_t* size)
    {
        *size = CORE_UTIL_READ_PROGMEM_BYTE(_languageString.header.size);
        return &_languageString;
    }

    const void* manufacturerString(uint16_t* size)
    {
        *size = CORE_UTIL_READ_PROGMEM_BYTE(_manufacturerString.header.size);
        return &_manufacturerString;
    }

    const void* productString(uint16_t* size)
    {
        *size = CORE_UTIL_READ_PROGMEM_BYTE(_productString.header.size);
        return &_productString;
    }
}    // namespace board::detail::usb
#endif
#endif