/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <inttypes.h>

///
/// \brief Bootloader special address to start the user application.
///
#define COMMAND_STARTAPPLICATION        0xFFFF

namespace bootloader
{
    ///
    /// Flag to indicate if the bootloader should be running, or should exit and allow the application code to run
    /// via a soft reset. When cleared, the bootloader will abort, the USB interface will shut down and the application
    /// started via a forced watchdog reset.
    ///
    extern bool RunBootloader;

    #if !defined(USB_SUPPORTED) || defined(BOARD_A_xu2)
    ///
    /// \brief Sequence used to signal that the USB link MCU has new flash page for target MCU.
    ///
    const uint8_t hidUploadStart[] =
    {
        0x08,
        0x04,
        0x07,
        0x02,
        0x84,
        0x72
    };
    #endif
}