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

#include "board/Board.h"
#include "board/Internal.h"
#include "core/src/Timing.h"
#include "core/src/MCU.h"
#ifdef USB_OVER_SERIAL
#include "board/common/communication/USBOverSerial/USBOverSerial.h"
#include "usb-link/Commands.h"
#endif

namespace Board
{
    void reboot()
    {
#ifndef USB_SUPPORTED
        // signal to usb link to reboot as well

        uint8_t data[2] = {
            static_cast<uint8_t>(USBLink::internalCMD_t::REBOOT_BTLDR),
            Board::bootloader::magicBootValue()
        };

        USBOverSerial::USBWritePacket packet(USBOverSerial::packetType_t::INTERNAL,
                                             data,
                                             2,
                                             USB_OVER_SERIAL_BUFFER_SIZE);
        USBOverSerial::write(UART_CHANNEL_USB_LINK, packet);

        while (!Board::UART::isTxComplete(UART_CHANNEL_USB_LINK))
        {
            ;
        }

        // give some time to usb link to properly re-initialize so that everything is in sync
        core::timing::waitMs(50);
#endif

        core::mcu::reset();
    }
}    // namespace Board