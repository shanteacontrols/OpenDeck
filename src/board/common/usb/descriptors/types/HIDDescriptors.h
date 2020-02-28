/*

Copyright 2015-2020 Igor Petrovic

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
  Copyright 2018  Dean Camera (dean [at] fourwalledcubicle [dot] com)

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

#define HID_IOF_CONSTANT           (1 << 0)
#define HID_IOF_DATA               (0 << 0)
#define HID_IOF_VARIABLE           (1 << 1)
#define HID_IOF_ARRAY              (0 << 1)
#define HID_IOF_RELATIVE           (1 << 2)
#define HID_IOF_ABSOLUTE           (0 << 2)
#define HID_IOF_WRAP               (1 << 3)
#define HID_IOF_NO_WRAP            (0 << 3)
#define HID_IOF_NON_LINEAR         (1 << 4)
#define HID_IOF_LINEAR             (0 << 4)
#define HID_IOF_NO_PREFERRED_STATE (1 << 5)
#define HID_IOF_PREFERRED_STATE    (0 << 5)
#define HID_IOF_NULLSTATE          (1 << 6)
#define HID_IOF_NO_NULL_POSITION   (0 << 6)
#define HID_IOF_VOLATILE           (1 << 7)
#define HID_IOF_NON_VOLATILE       (0 << 7)
#define HID_IOF_BUFFERED_BYTES     (1 << 8)
#define HID_IOF_BITFIELD           (0 << 8)

#define HID_RI_DATA_SIZE_MASK      0x03
#define HID_RI_TYPE_MASK           0x0C
#define HID_RI_TAG_MASK            0xF0
#define HID_RI_TYPE_MAIN           0x00
#define HID_RI_TYPE_GLOBAL         0x04
#define HID_RI_TYPE_LOCAL          0x08
#define HID_RI_DATA_BITS_0         0x00
#define HID_RI_DATA_BITS_8         0x01
#define HID_RI_DATA_BITS_16        0x02
#define HID_RI_DATA_BITS_32        0x03
#define HID_RI_DATA_BITS(DataBits) CONCAT_EXPANDED(HID_RI_DATA_BITS_, DataBits)

#define _HID_RI_ENTRY(Type, Tag, DataBits, ...) (Type | Tag | HID_RI_DATA_BITS(DataBits)) _HID_RI_ENCODE(DataBits, (__VA_ARGS__))

#define HID_RI_INPUT(DataBits, ...)            _HID_RI_ENTRY(HID_RI_TYPE_MAIN, 0x80, DataBits, __VA_ARGS__)
#define HID_RI_OUTPUT(DataBits, ...)           _HID_RI_ENTRY(HID_RI_TYPE_MAIN, 0x90, DataBits, __VA_ARGS__)
#define HID_RI_COLLECTION(DataBits, ...)       _HID_RI_ENTRY(HID_RI_TYPE_MAIN, 0xA0, DataBits, __VA_ARGS__)
#define HID_RI_FEATURE(DataBits, ...)          _HID_RI_ENTRY(HID_RI_TYPE_MAIN, 0xB0, DataBits, __VA_ARGS__)
#define HID_RI_END_COLLECTION(DataBits, ...)   _HID_RI_ENTRY(HID_RI_TYPE_MAIN, 0xC0, DataBits, __VA_ARGS__)
#define HID_RI_USAGE_PAGE(DataBits, ...)       _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0x00, DataBits, __VA_ARGS__)
#define HID_RI_LOGICAL_MINIMUM(DataBits, ...)  _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0x10, DataBits, __VA_ARGS__)
#define HID_RI_LOGICAL_MAXIMUM(DataBits, ...)  _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0x20, DataBits, __VA_ARGS__)
#define HID_RI_PHYSICAL_MINIMUM(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0x30, DataBits, __VA_ARGS__)
#define HID_RI_PHYSICAL_MAXIMUM(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0x40, DataBits, __VA_ARGS__)
#define HID_RI_UNIT_EXPONENT(DataBits, ...)    _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0x50, DataBits, __VA_ARGS__)
#define HID_RI_UNIT(DataBits, ...)             _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0x60, DataBits, __VA_ARGS__)
#define HID_RI_REPORT_SIZE(DataBits, ...)      _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0x70, DataBits, __VA_ARGS__)
#define HID_RI_REPORT_ID(DataBits, ...)        _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0x80, DataBits, __VA_ARGS__)
#define HID_RI_REPORT_COUNT(DataBits, ...)     _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0x90, DataBits, __VA_ARGS__)
#define HID_RI_PUSH(DataBits, ...)             _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0xA0, DataBits, __VA_ARGS__)
#define HID_RI_POP(DataBits, ...)              _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0xB0, DataBits, __VA_ARGS__)
#define HID_RI_USAGE(DataBits, ...)            _HID_RI_ENTRY(HID_RI_TYPE_LOCAL, 0x00, DataBits, __VA_ARGS__)
#define HID_RI_USAGE_MINIMUM(DataBits, ...)    _HID_RI_ENTRY(HID_RI_TYPE_LOCAL, 0x10, DataBits, __VA_ARGS__)
#define HID_RI_USAGE_MAXIMUM(DataBits, ...)    _HID_RI_ENTRY(HID_RI_TYPE_LOCAL, 0x20, DataBits, __VA_ARGS__)

// clang-format off
#define _HID_RI_ENCODE_0(Data)
#define _HID_RI_ENCODE_8(Data)                  , (Data & 0xFF)
#define _HID_RI_ENCODE_16(Data)                 _HID_RI_ENCODE_8(Data)  _HID_RI_ENCODE_8(Data >> 8)
#define _HID_RI_ENCODE_32(Data)                 _HID_RI_ENCODE_16(Data) _HID_RI_ENCODE_16(Data >> 16)
#define _HID_RI_ENCODE(DataBits, ...)           CONCAT_EXPANDED(_HID_RI_ENCODE_, DataBits(__VA_ARGS__))
// clang-format on

/** Enum for possible Class, Subclass and Protocol values of device and interface descriptors relating to the HID
 *  device class.
 */
enum HID_Descriptor_ClassSubclassProtocol_t
{
    HID_CSCP_HIDClass             = 0x03, /**< Descriptor Class value indicating that the device or interface
                                                   *   belongs to the HID class.
                                                   */
    HID_CSCP_NonBootSubclass      = 0x00, /**< Descriptor Subclass value indicating that the device or interface
                                                   *   does not implement a HID boot protocol.
                                                   */
    HID_CSCP_BootSubclass         = 0x01, /**< Descriptor Subclass value indicating that the device or interface
                                                   *   implements a HID boot protocol.
                                                   */
    HID_CSCP_NonBootProtocol      = 0x00, /**< Descriptor Protocol value indicating that the device or interface
                                                   *   does not belong to a HID boot protocol.
                                                   */
    HID_CSCP_KeyboardBootProtocol = 0x01, /**< Descriptor Protocol value indicating that the device or interface
                                                   *   belongs to the Keyboard HID boot protocol.
                                                   */
    HID_CSCP_MouseBootProtocol    = 0x02, /**< Descriptor Protocol value indicating that the device or interface
                                                   *   belongs to the Mouse HID boot protocol.
                                                   */
};

/** Enum for the HID class specific control requests that can be issued by the USB bus host. */
enum HID_ClassRequests_t
{
    HID_REQ_GetReport   = 0x01, /**< HID class-specific Request to get the current HID report from the device. */
    HID_REQ_GetIdle     = 0x02, /**< HID class-specific Request to get the current device idle count. */
    HID_REQ_GetProtocol = 0x03, /**< HID class-specific Request to get the current HID report protocol mode. */
    HID_REQ_SetReport   = 0x09, /**< HID class-specific Request to set the current HID report to the device. */
    HID_REQ_SetIdle     = 0x0A, /**< HID class-specific Request to set the device's idle count. */
    HID_REQ_SetProtocol = 0x0B, /**< HID class-specific Request to set the current HID report protocol mode. */
};

/** Enum for the HID class specific descriptor types. */
enum HID_DescriptorTypes_t
{
    HID_DTYPE_HID    = 0x21, /**< Descriptor header type value, to indicate a HID class HID descriptor. */
    HID_DTYPE_Report = 0x22, /**< Descriptor header type value, to indicate a HID class HID report descriptor. */
};

/** Enum for the different types of HID reports. */
enum HID_ReportItemTypes_t
{
    HID_REPORT_ITEM_In      = 0, /**< Indicates that the item is an IN report type. */
    HID_REPORT_ITEM_Out     = 1, /**< Indicates that the item is an OUT report type. */
    HID_REPORT_ITEM_Feature = 2, /**< Indicates that the item is a FEATURE report type. */
};

/** \brief HID class-specific HID Descriptor (LUFA naming conventions).
         *
         *  Type define for the HID class-specific HID descriptor, to describe the HID device's specifications. Refer to the HID
         *  specification for details on the structure elements.
         *
         *  \see \ref USB_HID_StdDescriptor_HID_t for the version of this type with standard element names.
         *
         *  \note Regardless of CPU architecture, these values should be stored as little endian.
         */
typedef struct
{
    USB_Descriptor_Header_t Header; /**< Regular descriptor header containing the descriptor's type and length. */

    uint16_t HIDSpec;     /**< BCD encoded version that the HID descriptor and device complies to.
                                              *
                                              *   \see \ref VERSION_BCD() utility macro.
                                              */
    uint8_t  CountryCode; /**< Country code of the localized device, or zero if universal. */

    uint8_t TotalReportDescriptors; /**< Total number of HID report descriptors for the interface. */

    uint8_t  HIDReportType;   /**< Type of HID report, set to \ref HID_DTYPE_Report. */
    uint16_t HIDReportLength; /**< Length of the associated HID report descriptor, in bytes. */
} ATTR_PACKED USB_HID_Descriptor_HID_t;

/** \brief HID class-specific HID Descriptor (USB-IF naming conventions).
         *
         *  Type define for the HID class-specific HID descriptor, to describe the HID device's specifications. Refer to the HID
         *  specification for details on the structure elements.
         *
         *  \see \ref USB_HID_Descriptor_HID_t for the version of this type with non-standard LUFA specific
         *       element names.
         *
         *  \note Regardless of CPU architecture, these values should be stored as little endian.
         */
typedef struct
{
    uint8_t bLength;         /**< Size of the descriptor, in bytes. */
    uint8_t bDescriptorType; /**< Type of the descriptor, either a value in \ref USB_DescriptorTypes_t or a value
                                       *   given by the specific class.
                                       */

    uint16_t bcdHID;       /**< BCD encoded version that the HID descriptor and device complies to.
                              *
                              *   \see \ref VERSION_BCD() utility macro.
                              */
    uint8_t  bCountryCode; /**< Country code of the localized device, or zero if universal. */

    uint8_t bNumDescriptors; /**< Total number of HID report descriptors for the interface. */

    uint8_t  bDescriptorType2;  /**< Type of HID report, set to \ref HID_DTYPE_Report. */
    uint16_t wDescriptorLength; /**< Length of the associated HID report descriptor, in bytes. */
} ATTR_PACKED USB_HID_StdDescriptor_HID_t;

/** Type define for the data type used to store HID report descriptor elements. */
typedef uint8_t USB_Descriptor_HIDReport_Datatype_t;