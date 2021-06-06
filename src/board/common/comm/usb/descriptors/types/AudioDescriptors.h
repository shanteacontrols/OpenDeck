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

#pragma once

#include <inttypes.h>
#include "StdDescriptors.h"

/** \brief Audio class-specific Interface Descriptor (LUFA naming conventions).
 *
 *  Type define for an Audio class-specific interface descriptor. This follows a regular interface descriptor to
 *  supply extra information about the audio device's layout to the host. See the USB Audio specification for more
 *  details.
 *
 *  \see \ref USB_Audio_StdDescriptor_Interface_AC_t for the version of this type with standard element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
    USB_Descriptor_Header_t Header;  /**< Regular descriptor header containing the descriptor's type and length. */
    uint8_t                 Subtype; /**< Sub type value used to distinguish between audio class-specific descriptors,
                                        *   a value from the \ref Audio_CSInterface_AS_SubTypes_t enum.
                                        */

    uint16_t ACSpecification; /**< Binary Coded Decimal value, indicating the supported Audio Class specification version.
                                                *
                                                *   \see \ref VERSION_BCD() utility macro.
                                                */
    uint16_t TotalLength;     /**< Total length of the Audio class-specific descriptors, including this descriptor. */

    uint8_t InCollection;    /**< Total number of Audio Streaming interfaces linked to this Audio Control interface (must be 1). */
    uint8_t InterfaceNumber; /**< Interface number of the associated Audio Streaming interface. */
} __attribute__((packed)) USB_Audio_Descriptor_Interface_AC_t;

/** \brief Audio class-specific Streaming Endpoint Descriptor (LUFA naming conventions).
 *
 *  Type define for an Audio class-specific endpoint descriptor. This contains a regular endpoint
 *  descriptor with a few Audio-class-specific extensions. See the USB Audio specification for more details.
 *
 *  \see \ref USB_Audio_StdDescriptor_StreamEndpoint_Std_t for the version of this type with standard element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
    USB_Descriptor_Endpoint_t Endpoint; /**< Standard endpoint descriptor describing the audio endpoint. */

    uint8_t Refresh;            /**< Always set to zero for Audio class devices. */
    uint8_t SyncEndpointNumber; /**< Endpoint address to send synchronization information to, if needed (zero otherwise). */
} __attribute__((packed)) USB_Audio_Descriptor_StreamEndpoint_Std_t;

enum Audio_Descriptor_ClassSubclassProtocol_t
{
    AUDIO_CSCP_AudioClass             = 0x01, /**< Descriptor Class value indicating that the device or
                                                        *   interface belongs to the USB Audio 1.0 class.
                                                        */
    AUDIO_CSCP_ControlSubclass        = 0x01, /**< Descriptor Subclass value indicating that the device or
                                                        *   interface belongs to the Audio Control subclass.
                                                        */
    AUDIO_CSCP_ControlProtocol        = 0x00, /**< Descriptor Protocol value indicating that the device or
                                                        *   interface belongs to the Audio Control protocol.
                                                        */
    AUDIO_CSCP_AudioStreamingSubclass = 0x02, /**< Descriptor Subclass value indicating that the device or
                                                        *   interface belongs to the MIDI Streaming subclass.
                                                        */
    AUDIO_CSCP_MIDIStreamingSubclass  = 0x03, /**< Descriptor Subclass value indicating that the device or
                                                        *   interface belongs to the Audio streaming subclass.
                                                        */
    AUDIO_CSCP_StreamingProtocol      = 0x00, /**< Descriptor Protocol value indicating that the device or
                                                        *   interface belongs to the Streaming Audio protocol.
                                                        */
};

/** Enum for the Audio class specific descriptor types. */
enum AUDIO_DescriptorTypes_t
{
    AUDIO_DTYPE_CSInterface = 0x24, /**< Audio class specific Interface functional descriptor. */
    AUDIO_DTYPE_CSEndpoint  = 0x25, /**< Audio class specific Endpoint functional descriptor. */
};

/** Audio class specific interface description subtypes, for the Audio Control interface. */
enum Audio_CSInterface_AC_SubTypes_t
{
    AUDIO_DSUBTYPE_CSInterface_Header         = 0x01, /**< Audio class specific control interface header. */
    AUDIO_DSUBTYPE_CSInterface_InputTerminal  = 0x02, /**< Audio class specific control interface Input Terminal. */
    AUDIO_DSUBTYPE_CSInterface_OutputTerminal = 0x03, /**< Audio class specific control interface Output Terminal. */
    AUDIO_DSUBTYPE_CSInterface_Mixer          = 0x04, /**< Audio class specific control interface Mixer Unit. */
    AUDIO_DSUBTYPE_CSInterface_Selector       = 0x05, /**< Audio class specific control interface Selector Unit. */
    AUDIO_DSUBTYPE_CSInterface_Feature        = 0x06, /**< Audio class specific control interface Feature Unit. */
    AUDIO_DSUBTYPE_CSInterface_Processing     = 0x07, /**< Audio class specific control interface Processing Unit. */
    AUDIO_DSUBTYPE_CSInterface_Extension      = 0x08, /**< Audio class specific control interface Extension Unit. */
};

/** Audio class specific interface description subtypes, for the Audio Streaming interface. */
enum Audio_CSInterface_AS_SubTypes_t
{
    AUDIO_DSUBTYPE_CSInterface_General        = 0x01, /**< Audio class specific streaming interface general descriptor. */
    AUDIO_DSUBTYPE_CSInterface_FormatType     = 0x02, /**< Audio class specific streaming interface format type descriptor. */
    AUDIO_DSUBTYPE_CSInterface_FormatSpecific = 0x03, /**< Audio class specific streaming interface format information descriptor. */
};

/** Audio class specific endpoint description subtypes, for the Audio Streaming interface. */
enum Audio_CSEndpoint_SubTypes_t
{
    AUDIO_DSUBTYPE_CSEndpoint_General = 0x01, /**< Audio class specific endpoint general descriptor. */
};