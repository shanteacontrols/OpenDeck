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

#ifdef PROJECT_TARGET_SUPPORT_USB
#ifdef BOARD_USE_TINYUSB

#include "internal.h"
#include "common/communication/usb/descriptors/common/common.h"

#include "tusb.h"
#include "core/mcu.h"

namespace
{
    volatile bool usbConnected;
}    // namespace

void core::mcu::isr::usb()
{
    tud_int_handler(0);
}

extern "C" uint8_t const* tud_descriptor_device_cb(void)
{
    uint16_t size = 0;
    auto     desc = board::detail::usb::deviceDescriptor(&size);
    return (uint8_t const*)desc;
}

extern "C" uint8_t const* tud_descriptor_configuration_cb(uint8_t index)
{
    uint16_t size = 0;
    auto     desc = board::detail::usb::cfgDescriptor(&size);
    return (uint8_t const*)desc;
}

extern "C" uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    uint16_t length;

    switch (index)
    {
    case USB_STRING_ID_LANGUAGE:
    {
        auto desc = board::detail::usb::languageString(&length);
        return (uint16_t*)desc;
    }
    break;

    case USB_STRING_ID_MANUFACTURER:
    {
        auto desc = board::detail::usb::manufacturerString(&length);
        return (uint16_t*)desc;
    }
    break;

    case USB_STRING_ID_PRODUCT:
    {
        auto desc = board::detail::usb::productString(&length);
        return (uint16_t*)desc;
    }
    break;

    case USB_STRING_ID_UID:
    {
        core::mcu::uniqueID_t uid;
        core::mcu::uniqueID(uid);

        auto desc = board::detail::usb::serialIDString(&length, &uid[0]);
        return (uint16_t*)desc;
    }
    break;

    default:
        return NULL;
    }
}

extern "C" void tud_mount_cb(void)
{
    usbConnected = true;
}

extern "C" void tud_umount_cb(void)
{
    usbConnected = false;
}

namespace board
{
    namespace usb
    {
        bool isUsbConnected()
        {
            return usbConnected;
        }
    }    // namespace usb

    namespace detail::usb
    {
        void deInit()
        {
            tud_disconnect();
        }

        void update()
        {
            if (board::usb::isInitialized())
            {
                tud_task();
            }
        }
    }    // namespace detail::usb
}    // namespace board

#endif
#endif