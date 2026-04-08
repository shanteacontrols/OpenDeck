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

#pragma once

#include "core/mcu.h"

#define USB_VENDOR_ID 0x1209

#if defined(OPENDECK_FW_APP)
#define USB_PRODUCT_ID 0x8472
#elif defined(OPENDECK_FW_BOOT)
#define USB_PRODUCT_ID 0x8473
#endif

#define USB_MANUFACTURER_NAME CORE_MCU_USB_STRING("Shantea Controls")