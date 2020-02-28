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

/** Marks a variable or struct element for packing into the smallest space available, omitting any
 *  alignment bytes usually added between fields to optimize field accesses.
 */
#define ATTR_PACKED __attribute__((packed))

/** Indicates the minimum alignment in bytes for a variable or struct element.
 *
 *  \param[in] Bytes  Minimum number of bytes the item should be aligned to.
 */
#define ATTR_ALIGNED(Bytes) __attribute__((aligned(Bytes)))

/** Macro to calculate the power value for the configuration descriptor, from a given number of milliamperes.
 *
 *  \param[in] mA  Maximum number of milliamps the device consumes when the given configuration is selected.
 */
#define USB_CONFIG_POWER_MA(mA) ((mA) >> 1)

/** Mask for the reserved bit in the Configuration Descriptor's \c ConfigAttributes field, which must be always
 *  set on all USB devices for historical purposes.
 */
#define USB_CONFIG_ATTR_RESERVED 0x80

/** Endpoint direction mask, for masking against endpoint addresses to retrieve the endpoint's
 *  direction for comparing with the \c ENDPOINT_DIR_* masks.
 */
#define ENDPOINT_DIR_MASK 0x80

/** Endpoint address direction mask for an OUT direction (Host to Device) endpoint. This may be ORed with
 *  the index of the address within a device to obtain the full endpoint address.
 */
#define ENDPOINT_DIR_OUT 0x00

/** Endpoint address direction mask for an IN direction (Device to Host) endpoint. This may be ORed with
 *  the index of the address within a device to obtain the full endpoint address.
 */
#define ENDPOINT_DIR_IN 0x80

/** Mask for determining the type of an endpoint from an endpoint descriptor. This should then be compared
 *  with the \c EP_TYPE_* masks to determine the exact type of the endpoint.
 */
#define EP_TYPE_MASK 0x03

/** Mask for a CONTROL type endpoint or pipe.
 *
 *  \note See \ref Group_EndpointManagement and \ref Group_PipeManagement for endpoint/pipe functions.
 */
#define EP_TYPE_CONTROL 0x00

/** Mask for an ISOCHRONOUS type endpoint or pipe.
 *
 *  \note See \ref Group_EndpointManagement and \ref Group_PipeManagement for endpoint/pipe functions.
 */
#define EP_TYPE_ISOCHRONOUS 0x01

/** Mask for a BULK type endpoint or pipe.
 *
 *  \note See \ref Group_EndpointManagement and \ref Group_PipeManagement for endpoint/pipe functions.
 */
#define EP_TYPE_BULK 0x02

/** Mask for an INTERRUPT type endpoint or pipe.
 *
 *  \note See \ref Group_EndpointManagement and \ref Group_PipeManagement for endpoint/pipe functions.
 */
#define EP_TYPE_INTERRUPT 0x03

/** Concatenates the given input into a single token, via the C Preprocessor.
 *
 *  \param[in] x  First item to concatenate.
 *  \param[in] y  Second item to concatenate.
 *
 *  \return Concatenated version of the input.
 */
#define CONCAT(x, y) x##y

/** Concatenates the given input into a single token after macro expansion, via the C Preprocessor.
 *
 *  \param[in] x  First item to concatenate.
 *  \param[in] y  Second item to concatenate.
 *
 *  \return Concatenated version of the expanded input.
 */
#define CONCAT_EXPANDED(x, y) CONCAT(x, y)