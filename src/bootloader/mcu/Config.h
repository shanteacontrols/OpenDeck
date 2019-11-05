/*

Copyright 2015-2019 Igor Petrovic

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

///
/// \brief Bootloader special address to start the user application.
///
#define COMMAND_STARTAPPLICATION 0xFFFF

namespace bootloader
{
    ///
    /// Flag to indicate if the bootloader should be running, or should exit and allow the application code to run
    /// via a soft reset. When cleared, the bootloader will abort, the USB interface will shut down and the application
    /// started via a forced watchdog reset.
    ///
    extern bool RunBootloader;

#if !defined(USB_MIDI_SUPPORTED) || (defined(OD_BOARD_16U2) || defined(OD_BOARD_8U2))
    ///
    /// \brief Sequence used to signal that the USB link MCU has new flash page for target MCU.
    ///
    const uint8_t hidUploadStart[] = {
        0x08,
        0x04,
        0x07,
        0x02,
        0x84,
        0x72
    };
#endif
}    // namespace bootloader