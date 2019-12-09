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

#ifdef USB_MIDI_SUPPORTED
#include "board/common/usb/descriptors/Descriptors.h"
#endif
#include "Config.h"
#include "core/src/general/Helpers.h"
#include "core/src/general/IO.h"
#include "core/src/general/Reset.h"
#include "core/src/general/Helpers.h"
#include "core/src/general/Timing.h"
#include "board/common/io/Helpers.h"
#include "board/common/constants/Reboot.h"
#include "Pins.h"
#include "board/Board.h"
#include "board/common/usb/descriptors/hid/Redef.h"
#include "common/OpenDeckMIDIformat/OpenDeckMIDIformat.h"
#include "variant/Variant.h"

namespace bootloader
{
    bool RunBootloader = true;
}

///
/// \brief Main program entry point.
/// This routine configures the hardware required by the bootloader, then continuously
/// runs the bootloader processing routine until instructed to soft-exit.
///
int main(void)
{
    Board::init();

    while (bootloader::RunBootloader)
    {
#ifdef USB_MIDI_SUPPORTED
        USB_USBTask();
#else
        static uint8_t pageStartCnt = 0;
        uint8_t        data;
        while (!Board::UART::read(UART_USB_LINK_CHANNEL, data))
            ;

        if (pageStartCnt < 6)
        {
            if (data == bootloader::hidUploadStart[pageStartCnt])
            {
                pageStartCnt++;

                if (pageStartCnt == 6)
                {
                    EVENT_UART_Device_ControlRequest();
                    pageStartCnt = 0;
                }
            }
            else
            {
                pageStartCnt = 0;
            }
        }
#endif
    }

    core::reset::mcuReset();
}