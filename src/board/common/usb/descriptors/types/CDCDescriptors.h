#pragma once

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

#include "StdDescriptors.h"

/** Enum for the CDC class specific control requests that can be issued by the USB bus host. */
enum CDC_ClassRequests_t
{
    CDC_REQ_SendEncapsulatedCommand = 0x00, /**< CDC class-specific request to send an encapsulated command to the device. */
    CDC_REQ_GetEncapsulatedResponse = 0x01, /**< CDC class-specific request to retrieve an encapsulated command response from the device. */
    CDC_REQ_SetLineEncoding         = 0x20, /**< CDC class-specific request to set the current virtual serial port configuration settings. */
    CDC_REQ_GetLineEncoding         = 0x21, /**< CDC class-specific request to get the current virtual serial port configuration settings. */
    CDC_REQ_SetControlLineState     = 0x22, /**< CDC class-specific request to set the current virtual serial port handshake line states. */
    CDC_REQ_SendBreak               = 0x23, /**< CDC class-specific request to send a break to the receiver via the carrier channel. */
};

/** Enum for the CDC class specific interface descriptor subtypes. */
enum CDC_DescriptorSubtypes_t
{
    CDC_DSUBTYPE_CSInterface_Header           = 0x00, /**< CDC class specific Header functional descriptor. */
    CDC_DSUBTYPE_CSInterface_CallManagement   = 0x01, /**< CDC class specific Call Management functional descriptor. */
    CDC_DSUBTYPE_CSInterface_ACM              = 0x02, /**< CDC class specific Abstract Control Model functional descriptor. */
    CDC_DSUBTYPE_CSInterface_DirectLine       = 0x03, /**< CDC class specific Direct Line functional descriptor. */
    CDC_DSUBTYPE_CSInterface_TelephoneRinger  = 0x04, /**< CDC class specific Telephone Ringer functional descriptor. */
    CDC_DSUBTYPE_CSInterface_TelephoneCall    = 0x05, /**< CDC class specific Telephone Call functional descriptor. */
    CDC_DSUBTYPE_CSInterface_Union            = 0x06, /**< CDC class specific Union functional descriptor. */
    CDC_DSUBTYPE_CSInterface_CountrySelection = 0x07, /**< CDC class specific Country Selection functional descriptor. */
    CDC_DSUBTYPE_CSInterface_TelephoneOpModes = 0x08, /**< CDC class specific Telephone Operation Modes functional descriptor. */
    CDC_DSUBTYPE_CSInterface_USBTerminal      = 0x09, /**< CDC class specific USB Terminal functional descriptor. */
    CDC_DSUBTYPE_CSInterface_NetworkChannel   = 0x0A, /**< CDC class specific Network Channel functional descriptor. */
    CDC_DSUBTYPE_CSInterface_ProtocolUnit     = 0x0B, /**< CDC class specific Protocol Unit functional descriptor. */
    CDC_DSUBTYPE_CSInterface_ExtensionUnit    = 0x0C, /**< CDC class specific Extension Unit functional descriptor. */
    CDC_DSUBTYPE_CSInterface_MultiChannel     = 0x0D, /**< CDC class specific Multi-Channel Management functional descriptor. */
    CDC_DSUBTYPE_CSInterface_CAPI             = 0x0E, /**< CDC class specific Common ISDN API functional descriptor. */
    CDC_DSUBTYPE_CSInterface_Ethernet         = 0x0F, /**< CDC class specific Ethernet functional descriptor. */
    CDC_DSUBTYPE_CSInterface_ATM              = 0x10, /**< CDC class specific Asynchronous Transfer Mode functional descriptor. */
};

/** \brief CDC class-specific Functional Header Descriptor (LUFA naming conventions).
*
*  Type define for a CDC class-specific functional header descriptor. This indicates to the host that the device
*  contains one or more CDC functional data descriptors, which give the CDC interface's capabilities and configuration.
*  See the CDC class specification for more details.
*
*  \see \ref USB_CDC_StdDescriptor_FunctionalHeader_t for the version of this type with standard element names.
*
*  \note Regardless of CPU architecture, these values should be stored as little endian.
*/
typedef struct
{
    USB_Descriptor_Header_t Header;           /**< Regular descriptor header containing the descriptor's type and length. */
    uint8_t                 Subtype;          /**< Sub type value used to distinguish between CDC class-specific descriptors,
                                                *   must be \ref CDC_DSUBTYPE_CSInterface_Header.
                                                */
    uint16_t                CDCSpecification; /**< Version number of the CDC specification implemented by the device,
                                                *   encoded in BCD format.
                                                *
                                                *   \see \ref VERSION_BCD() utility macro.
                                                */
} __attribute__((packed)) USB_CDC_Descriptor_FunctionalHeader_t;

/** \brief CDC class-specific Functional ACM Descriptor (LUFA naming conventions).
		 *
		 *  Type define for a CDC class-specific functional ACM descriptor. This indicates to the host that the CDC interface
		 *  supports the CDC ACM subclass of the CDC specification. See the CDC class specification for more details.
		 *
		 *  \see \ref USB_CDC_StdDescriptor_FunctionalACM_t for the version of this type with standard element names.
		 *
		 *  \note Regardless of CPU architecture, these values should be stored as little endian.
		 */
typedef struct
{
    USB_Descriptor_Header_t Header;       /**< Regular descriptor header containing the descriptor's type and length. */
    uint8_t                 Subtype;      /**< Sub type value used to distinguish between CDC class-specific descriptors,
			                                  *   must be \ref CDC_DSUBTYPE_CSInterface_ACM.
			                                  */
    uint8_t                 Capabilities; /**< Capabilities of the ACM interface, given as a bit mask. For most devices,
			                                       *   this should be set to a fixed value of \c 0x06 - for other capabilities, refer
			                                       *   to the CDC ACM specification.
			                                       */
} __attribute__((packed)) USB_CDC_Descriptor_FunctionalACM_t;

/** \brief CDC class-specific Functional Union Descriptor (LUFA naming conventions).
		 *
		 *  Type define for a CDC class-specific functional Union descriptor. This indicates to the host that specific
		 *  CDC control and data interfaces are related. See the CDC class specification for more details.
		 *
		 *  \see \ref USB_CDC_StdDescriptor_FunctionalUnion_t for the version of this type with standard element names.
		 *
		 *  \note Regardless of CPU architecture, these values should be stored as little endian.
		 */
typedef struct
{
    USB_Descriptor_Header_t Header;                /**< Regular descriptor header containing the descriptor's type and length. */
    uint8_t                 Subtype;               /**< Sub type value used to distinguish between CDC class-specific descriptors,
			                                  *   must be \ref CDC_DSUBTYPE_CSInterface_Union.
			                                  */
    uint8_t                 MasterInterfaceNumber; /**< Interface number of the CDC Control interface. */
    uint8_t                 SlaveInterfaceNumber;  /**< Interface number of the CDC Data interface. */
} __attribute__((packed)) USB_CDC_Descriptor_FunctionalUnion_t;

/** Enum for possible Class, Subclass and Protocol values of device and interface descriptors relating to the CDC
    *  device class.
    */
enum CDC_Descriptor_ClassSubclassProtocol_t
{
    CDC_CSCP_CDCClass               = 0x02, /**< Descriptor Class value indicating that the device or interface
			                                         *   belongs to the CDC class.
			                                         */
    CDC_CSCP_NoSpecificSubclass     = 0x00, /**< Descriptor Subclass value indicating that the device or interface
			                                         *   belongs to no specific subclass of the CDC class.
			                                         */
    CDC_CSCP_ACMSubclass            = 0x02, /**< Descriptor Subclass value indicating that the device or interface
			                                         *   belongs to the Abstract Control Model CDC subclass.
			                                         */
    CDC_CSCP_ATCommandProtocol      = 0x01, /**< Descriptor Protocol value indicating that the device or interface
			                                         *   belongs to the AT Command protocol of the CDC class.
			                                         */
    CDC_CSCP_NoSpecificProtocol     = 0x00, /**< Descriptor Protocol value indicating that the device or interface
			                                         *   belongs to no specific protocol of the CDC class.
			                                         */
    CDC_CSCP_VendorSpecificProtocol = 0xFF, /**< Descriptor Protocol value indicating that the device or interface
			                                         *   belongs to a vendor-specific protocol of the CDC class.
			                                         */
    CDC_CSCP_CDCDataClass           = 0x0A, /**< Descriptor Class value indicating that the device or interface
			                                         *   belongs to the CDC Data class.
			                                         */
    CDC_CSCP_NoDataSubclass         = 0x00, /**< Descriptor Subclass value indicating that the device or interface
			                                         *   belongs to no specific subclass of the CDC data class.
			                                         */
    CDC_CSCP_NoDataProtocol         = 0x00, /**< Descriptor Protocol value indicating that the device or interface
			                                         *   belongs to no specific protocol of the CDC data class.
			                                         */
};

/** Enum for the CDC class specific descriptor types. */
enum CDC_DescriptorTypes_t
{
    CDC_DTYPE_CSInterface = 0x24, /**< CDC class specific Interface functional descriptor. */
    CDC_DTYPE_CSEndpoint  = 0x25, /**< CDC class specific Endpoint functional descriptor. */
};