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

#include <inttypes.h>

namespace usbLink
{
    /// List of all internal commands when using USB MIDI over Serial protocol
    /// Enumeration value should be placed in Event field of USB MIDI packet
    enum class internalCMD_t : uint8_t
    {
        REBOOT_BTLDR,
        USB_STATE,
        BAUDRATE_CHANGE,
        UNIQUE_ID,
        DISCONNECT_USB,
        CONNECT_USB,
        LINK_READY
    };
}    // namespace usbLink