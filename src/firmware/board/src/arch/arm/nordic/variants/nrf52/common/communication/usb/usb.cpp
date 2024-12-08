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
#include "nrfx_power.h"
#include "nrf_sdh.h"
#include "nrf_sdm.h"

// tinyusb function that handles power event (detected, ready, removed)
// We must call it within SD's SOC event handler, or set it as power event handler if SD is not enabled.
extern "C" void tusb_hal_nrf_power_event(uint32_t event);

extern "C" void power_event_handler(nrfx_power_usb_evt_t event)
{
    tusb_hal_nrf_power_event((uint32_t)event);
}

namespace board::detail::usb
{
    void init()
    {
        NVIC_SetPriority(USBD_IRQn, IRQ_PRIORITY_USB);

        // USB power may already be ready at this time in which case no event is generated.
        // We need to invoke the handler based on the initial status.
        uint32_t usbReg = 0;

        if (nrf_sdh_is_enabled())
        {
            sd_power_usbdetected_enable(true);
            sd_power_usbpwrrdy_enable(true);
            sd_power_usbremoved_enable(true);

            sd_power_usbregstatus_get(&usbReg);
        }
        else
        {
            const nrfx_power_config_t PWR_CFG = { 0 };
            nrfx_power_init(&PWR_CFG);

            // register tusb function as USB power handler
            const nrfx_power_usbevt_config_t CONFIG = { .handler = power_event_handler };
            nrfx_power_usbevt_init(&CONFIG);

            nrfx_power_usbevt_enable();

            usbReg = NRF_POWER->USBREGSTATUS;
        }

        if (usbReg & POWER_USBREGSTATUS_VBUSDETECT_Msk)
        {
            tusb_hal_nrf_power_event(NRFX_POWER_USB_EVT_DETECTED);
        }

        if (usbReg & POWER_USBREGSTATUS_OUTPUTRDY_Msk)
        {
            tusb_hal_nrf_power_event(NRFX_POWER_USB_EVT_READY);
        }

        tusb_init();
        detail::registerUpdateHook(&board::detail::usb::update);
    }
}    // namespace board::detail::usb

#endif