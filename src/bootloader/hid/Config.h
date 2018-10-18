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
    //magic sequence used to signal that the usb link has new flash page for target mcu
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