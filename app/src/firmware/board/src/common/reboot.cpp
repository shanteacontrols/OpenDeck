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

#include "board/board.h"
#include "internal.h"

#include "core/mcu.h"

namespace board
{
    void reboot()
    {
        usb::deInit();

#ifndef PROJECT_TARGET_SUPPORT_USB
        // signal to usb link to reboot as well

        uint32_t magicVal = board::bootloader::magicBootValue();

        uint8_t data[5] = {
            static_cast<uint8_t>(usb_over_serial::internalCmd_t::REBOOT_BTLDR),
            static_cast<uint8_t>(magicVal >> 24 & 0xFF),
            static_cast<uint8_t>(magicVal >> 16 & 0xFF),
            static_cast<uint8_t>(magicVal >> 8 & 0xFF),
            static_cast<uint8_t>(magicVal >> 0 & 0xFF),
        };

        usb_over_serial::UsbWritePacket packet(usb_over_serial::packetType_t::INTERNAL,
                                               data,
                                               sizeof(data),
                                               sizeof(data));
        usb_over_serial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, packet);
#endif

        // In case the indicator LEDs were on before this command was issued, this will make sure
        // they are off before the reboot.
        // Double the delay time to avoid "sharp" transition between traffic event and bootloader indication.
        core::mcu::timing::waitMs(io::indicators::LED_TRAFFIC_INDICATOR_TIMEOUT * 2);

        core::mcu::reset();
    }
}    // namespace board