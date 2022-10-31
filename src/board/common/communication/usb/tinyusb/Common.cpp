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

#ifdef HW_SUPPORT_USB
#ifdef USE_TINYUSB

#include "board/Internal.h"
#include "board/common/communication/usb/descriptors/common/Common.h"
#include "tusb.h"
#include "core/src/MCU.h"

namespace
{
    volatile bool _usbConnected;
}    // namespace

void core::mcu::isr::usb()
{
    tud_int_handler(0);
}

extern "C" uint8_t const* tud_descriptor_device_cb(void)
{
    uint16_t size = 0;
    auto     desc = Board::detail::USB::deviceDescriptor(&size);
    return (uint8_t const*)desc;
}

extern "C" uint8_t const* tud_descriptor_configuration_cb(uint8_t index)
{
    uint16_t size = 0;
    auto     desc = Board::detail::USB::cfgDescriptor(&size);
    return (uint8_t const*)desc;
}

extern "C" uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    uint16_t length;

    switch (index)
    {
    case USB_STRING_ID_LANGUAGE:
    {
        auto desc = Board::detail::USB::languageString(&length);
        return (uint16_t*)desc;
    }
    break;

    case USB_STRING_ID_MANUFACTURER:
    {
        auto desc = Board::detail::USB::manufacturerString(&length);
        return (uint16_t*)desc;
    }
    break;

    case USB_STRING_ID_PRODUCT:
    {
        auto desc = Board::detail::USB::productString(&length);
        return (uint16_t*)desc;
    }
    break;

    case USB_STRING_ID_UID:
    {
        core::mcu::uniqueID_t uid;
        core::mcu::uniqueID(uid);

        auto desc = Board::detail::USB::serialIDString(&length, &uid[0]);
        return (uint16_t*)desc;
    }
    break;

    default:
        return NULL;
    }
}

extern "C" void tud_mount_cb(void)
{
    _usbConnected = true;
}

extern "C" void tud_umount_cb(void)
{
    _usbConnected = false;
}

namespace Board
{
    namespace USB
    {
        bool isUSBconnected()
        {
            return _usbConnected;
        }
    }    // namespace USB

    namespace detail::USB
    {
        void deInit()
        {
            tud_disconnect();
        }

        void update()
        {
            if (Board::USB::isInitialized())
            {
                tud_task();
            }
        }
    }    // namespace detail::USB
}    // namespace Board

#endif
#endif