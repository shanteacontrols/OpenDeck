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

#pragma once

#include "core/src/MCU.h"
#include "Endpoints.h"

struct usbDescriptorConfiguration_t
{
    core::mcu::usb::descriptorConfigurationHeader_t    config;
    core::mcu::usb::descriptorInterfaceAssociation_t   cdcIad;    // for composite usb device
    core::mcu::usb::descriptorInterface_t              cdcCciInterface;
    core::mcu::usb::cdcDescriptorFunctionalHeader_t    cdcFunctionalHeader;
    core::mcu::usb::cdcDescriptorFunctionalACM_t       cdcFunctionalAcm;
    core::mcu::usb::cdcDescriptorFunctionalUnion_t     cdcFunctionalUnion;
    core::mcu::usb::descriptorEndpoint_t               cdcNotificationEndpoint;
    core::mcu::usb::descriptorInterface_t              cdcDciInterface;
    core::mcu::usb::descriptorEndpoint_t               cdcDataOutEndpoint;
    core::mcu::usb::descriptorEndpoint_t               cdcDataInEndpoint;
    core::mcu::usb::descriptorInterface_t              audioControlInterface;
    core::mcu::usb::audioDescriptorInterfaceAC_t       audioControlInterfaceSpc;
    core::mcu::usb::descriptorInterface_t              audioStreamInterface;
    core::mcu::usb::midiDescriptorAudioInterfaceAS_t   audioStreamInterfaceSpc;
    core::mcu::usb::midiDescriptorInputJack_t          midiInJackEmb;
    core::mcu::usb::midiDescriptorInputJack_t          midiInJackExt;
    core::mcu::usb::midiDescriptorOutputJack_t         midiOutJackEmb;
    core::mcu::usb::midiDescriptorOutputJack_t         midiOutJackExt;
    core::mcu::usb::audioDescriptorStreamEndpointStd_t midiInJackEndPoint;
    core::mcu::usb::midiDescriptorJackEndpoint_t       midiInJackEndPointSpc;
    core::mcu::usb::audioDescriptorStreamEndpointStd_t midiOutJackEndpoint;
    core::mcu::usb::midiDescriptorJackEndpoint_t       midiOutJackEndpointSpc;
};

enum usbInterfaceDescriptor_t
{
    USB_INTERFACE_ID_CDC_CCI       = 0,
    USB_INTERFACE_ID_CDC_DCI       = 1,
    USB_INTERFACE_ID_AUDIO_CONTROL = 2,
    USB_INTERFACE_ID_AUDIO_STREAM  = 3,
};