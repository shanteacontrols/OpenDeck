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
#ifdef HW_USB_OVER_SERIAL
#include "board/common/communication/USBOverSerial/USBOverSerial.h"
#include "usb-link/Commands.h"
#endif

namespace board
{
    void reboot()
    {
        usb::deInit();

#ifndef HW_SUPPORT_USB
        // signal to usb link to reboot as well

        uint8_t data[2] = {
            static_cast<uint8_t>(usbLink::internalCMD_t::REBOOT_BTLDR),
            board::bootloader::magicBootValue()
        };

        usbOverSerial::USBWritePacket packet(usbOverSerial::packetType_t::INTERNAL,
                                             data,
                                             2,
                                             BUFFER_SIZE_USB_OVER_SERIAL);
        usbOverSerial::write(HW_UART_CHANNEL_USB_LINK, packet);
#endif

        // In case the indicator LEDs were on before this command was issued, this will make sure
        // they are off before the reboot.
        // Double the delay time to avoid "sharp" transition between traffic event and bootloader indication.
        core::timing::waitMs(io::indicators::LED_TRAFFIC_INDICATOR_TIMEOUT * 2);

        core::mcu::reset();
    }
}    // namespace board