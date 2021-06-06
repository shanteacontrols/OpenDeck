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

/** \brief MIDI class-specific Streaming Interface Descriptor (LUFA naming conventions).
 *
 *  Type define for an Audio class-specific MIDI streaming interface descriptor. This indicates to the host
 *  how MIDI the specification compliance of the device and the total length of the Audio class-specific descriptors.
 *  See the USB Audio specification for more details.
 *
 *  \see \ref USB_MIDI_StdDescriptor_AudioInterface_AS_t for the version of this type with standard element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
    USB_Descriptor_Header_t Header;  /**< Regular descriptor header containing the descriptor's type and length. */
    uint8_t                 Subtype; /**< Sub type value used to distinguish between audio class-specific descriptors. */

    uint16_t AudioSpecification; /**< Binary coded decimal value, indicating the supported Audio Class
                                                    *   specification version.
                                                    *
                                                    *   \see \ref VERSION_BCD() utility macro.
                                                    */
    uint16_t TotalLength;        /**< Total length of the Audio class-specific descriptors, including this descriptor. */
} __attribute__((packed)) USB_MIDI_Descriptor_AudioInterface_AS_t;

/** \brief MIDI class-specific Input Jack Descriptor (LUFA naming conventions).
 *
 *  Type define for an Audio class-specific MIDI IN jack. This gives information to the host on a MIDI input, either
 *  a physical input jack, or a logical jack (receiving input data internally, or from the host via an endpoint).
 *
 *  \see \ref USB_MIDI_StdDescriptor_InputJack_t for the version of this type with standard element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
    USB_Descriptor_Header_t Header;  /**< Regular descriptor header containing the descriptor's type and length. */
    uint8_t                 Subtype; /**< Sub type value used to distinguish between audio class-specific descriptors. */

    uint8_t JackType; /**< Type of jack, one of the \c JACKTYPE_* mask values. */
    uint8_t JackID;   /**< ID value of this jack - must be a unique value within the device. */

    uint8_t JackStrIndex; /**< Index of a string descriptor describing this descriptor within the device. */
} __attribute__((packed)) USB_MIDI_Descriptor_InputJack_t;

/** \brief MIDI class-specific Output Jack Descriptor (LUFA naming conventions).
 *
 *  Type define for an Audio class-specific MIDI OUT jack. This gives information to the host on a MIDI output, either
 *  a physical output jack, or a logical jack (sending output data internally, or to the host via an endpoint).
 *
 *  \see \ref USB_MIDI_StdDescriptor_OutputJack_t for the version of this type with standard element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
    USB_Descriptor_Header_t Header;  /**< Regular descriptor header containing the descriptor's type and length. */
    uint8_t                 Subtype; /**< Sub type value used to distinguish between audio class-specific descriptors. */

    uint8_t JackType; /**< Type of jack, one of the \c JACKTYPE_* mask values. */
    uint8_t JackID;   /**< ID value of this jack - must be a unique value within the device. */

    uint8_t NumberOfPins;    /**< Number of output channels within the jack, either physical or logical. */
    uint8_t SourceJackID[1]; /**< ID of each output pin's source data jack. */
    uint8_t SourcePinID[1];  /**< Pin number in the input jack of each output pin's source data. */

    uint8_t JackStrIndex; /**< Index of a string descriptor describing this descriptor within the device. */
} __attribute__((packed)) USB_MIDI_Descriptor_OutputJack_t;

/** \brief Audio class-specific Jack Endpoint Descriptor (LUFA naming conventions).
 *
 *  Type define for an Audio class-specific extended MIDI jack endpoint descriptor. This contains extra information
 *  on the usage of MIDI endpoints used to stream MIDI events in and out of the USB Audio device, and follows an Audio
 *  class-specific extended MIDI endpoint descriptor. See the USB Audio specification for more details.
 *
 *  \see \ref USB_MIDI_StdDescriptor_Jack_Endpoint_t for the version of this type with standard element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
    USB_Descriptor_Header_t Header;  /**< Regular descriptor header containing the descriptor's type and length. */
    uint8_t                 Subtype; /**< Sub type value used to distinguish between audio class-specific descriptors. */

    uint8_t TotalEmbeddedJacks;  /**< Total number of jacks inside this endpoint. */
    uint8_t AssociatedJackID[1]; /**< IDs of each jack inside the endpoint. */
} __attribute__((packed)) USB_MIDI_Descriptor_Jack_Endpoint_t;

/** Enum for the possible MIDI jack types in a MIDI device jack descriptor. */
enum MIDI_JackTypes_t
{
    MIDI_JACKTYPE_Embedded = 0x01, /**< MIDI class descriptor jack type value for an embedded (logical) MIDI input or output jack. */
    MIDI_JACKTYPE_External = 0x02, /**< MIDI class descriptor jack type value for an external (physical) MIDI input or output jack. */
};