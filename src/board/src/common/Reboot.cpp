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
#include "board/src/Internal.h"
#include "core/Timing.h"
#include "core/MCU.h"
#ifdef PROJECT_TARGET_USB_OVER_SERIAL
#include "board/src/common/communication/USBOverSerial/USBOverSerial.h"
#endif

namespace board
{
    void reboot()
    {
        usb::deInit();

#ifndef PROJECT_TARGET_SUPPORT_USB
        // signal to usb link to reboot as well

        uint32_t magicVal = board::bootloader::magicBootValue();

        uint8_t data[5] = {
            static_cast<uint8_t>(usbOverSerial::internalCMD_t::REBOOT_BTLDR),
            static_cast<uint8_t>(magicVal >> 24 & 0xFF),
            static_cast<uint8_t>(magicVal >> 16 & 0xFF),
            static_cast<uint8_t>(magicVal >> 8 & 0xFF),
            static_cast<uint8_t>(magicVal >> 0 & 0xFF),
        };

        usbOverSerial::USBWritePacket packet(usbOverSerial::packetType_t::INTERNAL,
                                             data,
                                             sizeof(data),
                                             sizeof(data));
        usbOverSerial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, packet);
#endif

        // In case the indicator LEDs were on before this command was issued, this will make sure
        // they are off before the reboot.
        // Double the delay time to avoid "sharp" transition between traffic event and bootloader indication.
        core::timing::waitMs(io::indicators::LED_TRAFFIC_INDICATOR_TIMEOUT * 2);

        core::mcu::reset();
    }
}    // namespace board