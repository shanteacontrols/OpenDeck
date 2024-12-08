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

#include "internal.h"

#include "tusb.h"
#include "core/mcu.h"

namespace board::detail::usb
{
    void init()
    {
        auto descriptor = core::mcu::peripherals::usbDescriptor();

        if (descriptor != nullptr)
        {
            descriptor->enableClock();

            for (size_t i = 0; i < descriptor->pins().size(); i++)
            {
                CORE_MCU_IO_INIT(descriptor->pins().at(i));
            }

            // disable VBUS sensing
            USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_NOVBUSSENS;

            tusb_init();
            detail::registerUpdateHook(&board::detail::usb::update);
        }
    }
}    // namespace board::detail::usb

#endif