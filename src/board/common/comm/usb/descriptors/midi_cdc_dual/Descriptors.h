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

#pragma once

#include "board/common/comm/usb/descriptors/Arch.h"
#include <comm/usb/midi_cdc_dual/Endpoints.h>

#define CDC_POLLING_TIME  5
#define USB_TX_TIMEOUT_MS 2000

typedef struct
{
    USB_Descriptor_Configuration_Header_t Config;

    // CDC Control Interface
    USB_Descriptor_Interface_Association_t CDC_IAD;    // for composite usb device
    USB_Descriptor_Interface_t             CDC_CCI_Interface;
    USB_CDC_Descriptor_FunctionalHeader_t  CDC_Functional_Header;
    USB_CDC_Descriptor_FunctionalACM_t     CDC_Functional_ACM;
    USB_CDC_Descriptor_FunctionalUnion_t   CDC_Functional_Union;
    USB_Descriptor_Endpoint_t              CDC_NotificationEndpoint;

    // CDC Data Interface
    USB_Descriptor_Interface_t CDC_DCI_Interface;
    USB_Descriptor_Endpoint_t  CDC_DataOutEndpoint;
    USB_Descriptor_Endpoint_t  CDC_DataInEndpoint;

    // MIDI Audio Control Interface
    USB_Descriptor_Interface_t          Audio_ControlInterface;
    USB_Audio_Descriptor_Interface_AC_t Audio_ControlInterface_SPC;

    // MIDI Audio Streaming Interface
    USB_Descriptor_Interface_t                Audio_StreamInterface;
    USB_MIDI_Descriptor_AudioInterface_AS_t   Audio_StreamInterface_SPC;
    USB_MIDI_Descriptor_InputJack_t           MIDI_In_Jack_Emb;
    USB_MIDI_Descriptor_InputJack_t           MIDI_In_Jack_Ext;
    USB_MIDI_Descriptor_OutputJack_t          MIDI_Out_Jack_Emb;
    USB_MIDI_Descriptor_OutputJack_t          MIDI_Out_Jack_Ext;
    USB_Audio_Descriptor_StreamEndpoint_Std_t MIDI_In_Jack_Endpoint;
    USB_MIDI_Descriptor_Jack_Endpoint_t       MIDI_In_Jack_Endpoint_SPC;
    USB_Audio_Descriptor_StreamEndpoint_Std_t MIDI_Out_Jack_Endpoint;
    USB_MIDI_Descriptor_Jack_Endpoint_t       MIDI_Out_Jack_Endpoint_SPC;
} USB_Descriptor_Configuration_t;

/** Enum for the device interface descriptor IDs within the device. Each interface descriptor
 *  should have a unique ID index associated with it, which can be used to refer to the
 *  interface from other descriptors.
 */
enum InterfaceDescriptors_t
{
    INTERFACE_ID_CDC_CCI      = 0, /**< CDC CCI interface descriptor ID */
    INTERFACE_ID_CDC_DCI      = 1, /**< CDC DCI interface descriptor ID */
    INTERFACE_ID_AudioControl = 2, /**< Audio control interface descriptor ID */
    INTERFACE_ID_AudioStream  = 3, /**< Audio stream interface descriptor ID */
};

/** Enum for the device string descriptor IDs within the device. Each string descriptor should
 *  have a unique ID index associated with it, which can be used to refer to the string from
 *  other descriptors.
 */
enum StringDescriptors_t
{
    STRING_ID_Language     = 0,                  /**< Supported Languages string descriptor ID (must be zero) */
    STRING_ID_Manufacturer = 1,                  /**< Manufacturer string ID */
    STRING_ID_Product      = 2,                  /**< Product string ID */
    STRING_ID_UID          = USE_INTERNAL_SERIAL /**< Unique serial number string ID */
};