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

#define _TUSB_CONFIG_H_

#pragma once

#ifdef FW_APP
#include "board/common/communication/usb/descriptors/midi_cdc_dual/Endpoints.h"
#define CFG_TUD_CDC 1
#else
#include "board/common/communication/usb/descriptors/midi/Endpoints.h"
#define CFG_TUD_CDC 0
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN __attribute__((aligned(4)))
#endif

#define BOARD_DEVICE_RHPORT_NUM   0
#define BOARD_DEVICE_RHPORT_SPEED OPT_MODE_FULL_SPEED
#define CFG_TUSB_RHPORT0_MODE     (OPT_MODE_DEVICE | BOARD_DEVICE_RHPORT_SPEED)
#define CFG_TUSB_OS               OPT_OS_NONE
#define CFG_TUD_ENDPOINT0_SIZE    USB_ENDPOINT_SIZE_CONTROL
#define CFG_TUD_CDC_EP_BUFSIZE    USB_ENDPOINT_SIZE_CDC_IN_OUT
#define CFG_TUD_MIDI_EP_BUFSIZE   USB_ENDPOINT_SIZE_MIDI_IN_OUT
#define CFG_TUD_MIDI_TX_BUFSIZE   USB_MIDI_TX_BUFFER_SIZE
#define CFG_TUD_MIDI_RX_BUFSIZE   USB_MIDI_RX_BUFFER_SIZE
#define CFG_TUD_CDC_TX_BUFSIZE    USB_CDC_TX_BUFFER_SIZE
#define CFG_TUD_CDC_RX_BUFSIZE    USB_CDC_RX_BUFFER_SIZE
#define CFG_TUD_MSC               0
#define CFG_TUD_HID               0
#define CFG_TUD_MIDI              1
#define CFG_TUD_VENDOR            0

#ifdef __cplusplus
}
#endif