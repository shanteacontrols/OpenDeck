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

#ifdef USE_CUSTOM_USB_ENDPOINTS
#include <communication/usb/midi_cdc_dual/Endpoints.h>
#else
#define USB_ENDPOINT_ADDR_CDC_IN           (core::mcu::usb::ENDPOINT_DIR_IN | 1)
#define USB_ENDPOINT_ADDR_MIDI_IN          (core::mcu::usb::ENDPOINT_DIR_IN | 2)
#define USB_ENDPOINT_ADDR_CDC_NOTIFICATION (core::mcu::usb::ENDPOINT_DIR_IN | 3)
#define USB_ENDPOINT_ADDR_CDC_OUT          (core::mcu::usb::ENDPOINT_DIR_OUT | 1)
#define USB_ENDPOINT_ADDR_MIDI_OUT         (core::mcu::usb::ENDPOINT_DIR_OUT | 2)
#define USB_ENDPOINT_SIZE_CONTROL          64
#define USB_ENDPOINT_SIZE_MIDI_IN_OUT      32
#define USB_ENDPOINT_SIZE_CDC_NOTIFICATION 8
#define USB_ENDPOINT_SIZE_CDC_IN_OUT       32
#endif