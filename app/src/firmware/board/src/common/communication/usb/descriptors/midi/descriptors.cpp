/*

Copyright Igor Petrovic

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

#include "internal.h"
#include "descriptors.h"
#include "common/communication/usb/constants.h"
#include "common/communication/usb/descriptors/common/common.h"
#include <target.h>

#include <string.h>
#include <stdio.h>

namespace
{
    const usbDescriptorConfiguration_t CONFIGURATION_DESCRIPTOR = {
        .config = {
            .header = {
                .size = sizeof(core::mcu::usb::descriptorConfigurationHeader_t),
                .type = core::mcu::usb::DESC_TYPE_CONFIGURATION,
            },

            .totalConfigurationSize = sizeof(usbDescriptorConfiguration_t),
            .totalInterfaces        = 2,
            .configurationNumber    = 1,
            .configurationStrIndex  = core::mcu::usb::NO_DESCRIPTOR,
            .configAttributes       = core::mcu::usb::CONF_DESC_ATTR_RESERVED,
            .maxPowerConsumption    = core::mcu::usb::CONF_DESC_POWER_MA(100),
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

                .endpointAddress   = PROJECT_MCU_USB_ENDPOINT_MIDI_ADDR_OUT,
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

                .endpointAddress   = PROJECT_MCU_USB_ENDPOINT_MIDI_ADDR_IN,
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

    const core::mcu::usb::descriptorDevice_t DEVICE_DESCRIPTOR = {
        .header = {
            .size = sizeof(core::mcu::usb::descriptorDevice_t),
            .type = core::mcu::usb::DESC_TYPE_DEVICE,
        },

        .usbSpecification       = core::mcu::usb::VERSION_BCD(2, 0, 0),
        .classId                = core::mcu::usb::CSCP_NO_DEVICE_CLASS,
        .subClassId             = core::mcu::usb::CSCP_NO_DEVICE_SUBCLASS,
        .protocol               = core::mcu::usb::CSCP_NO_DEVICE_PROTOCOL,
        .endpoint0Size          = PROJECT_MCU_USB_ENDPOINT_SIZE_CONTROL,
        .vendorId               = USB_VENDOR_ID,
        .productId              = USB_PRODUCT_ID,
        .releaseNumber          = core::mcu::usb::VERSION_BCD(0, 0, 1),
        .manufacturerStrIndex   = USB_STRING_ID_MANUFACTURER,
        .productStrIndex        = USB_STRING_ID_PRODUCT,
        .serialNumStrIndex      = USB_STRING_ID_UID,
        .numberOfConfigurations = 1
    };

    const core::mcu::usb::descriptorString_t LANGUAGE_STRING     = CORE_MCU_USB_STRING_DESCRIPTOR_ARRAY(core::mcu::usb::LANGUAGE_ID_ENG);
    const core::mcu::usb::descriptorString_t MANUFACTURER_STRING = CORE_MCU_USB_STRING_DESCRIPTOR(USB_MANUFACTURER_NAME);
    const core::mcu::usb::descriptorString_t PRODUCT_STRING      = CORE_MCU_USB_STRING_DESCRIPTOR(PROJECT_TARGET_USB_NAME);
}    // namespace

namespace board::detail::usb
{
    const void* cfgDescriptor(uint16_t* size)
    {
        *size = sizeof(usbDescriptorConfiguration_t);
        return &CONFIGURATION_DESCRIPTOR;
    }

    const void* deviceDescriptor(uint16_t* size)
    {
        *size = sizeof(core::mcu::usb::descriptorDevice_t);
        return &DEVICE_DESCRIPTOR;
    }

    const void* languageString(uint16_t* size)
    {
        *size = LANGUAGE_STRING.header.size;
        return &LANGUAGE_STRING;
    }

    const void* manufacturerString(uint16_t* size)
    {
        *size = MANUFACTURER_STRING.header.size;
        return &MANUFACTURER_STRING;
    }

    const void* productString(uint16_t* size)
    {
        *size = PRODUCT_STRING.header.size;
        return &PRODUCT_STRING;
    }
}    // namespace board::detail::usb
#endif