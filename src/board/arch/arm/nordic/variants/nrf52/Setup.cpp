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
#include "nrfx.h"
#include "nrf_sdm.h"
#include "nrfx_clock.h"
#include "nrfx_saadc.h"
#include "nrf_gpio.h"
#include "nrfx_power.h"
#include "nrf_nvmc.h"
#include "core/src/general/ADC.h"
#ifdef USB_SUPPORTED
#include "tusb.h"
#endif
#include <Target.h>

#ifdef ADC_SUPPORTED

namespace
{
    // nrf requires setting value from ram as an buffer in which adc value will be stored
    nrf_saadc_value_t _adcValue;
}    // namespace
#endif

// Tinyusb function that handles power event (detected, ready, removed).
// We must call it within SD's SOC event handler, or set it as power event handler if SD is not enabled.
extern "C" void tusb_hal_nrf_power_event(uint32_t event);

extern "C" void power_event_handler(nrfx_power_usb_evt_t event)
{
    tusb_hal_nrf_power_event((uint32_t)event);
}

namespace Board::detail::setup
{
    void halInit()
    {
        // always set voltage mode to 3V (if not already set)
        if ((NRF_UICR->REGOUT0 & UICR_REGOUT0_VOUT_Msk) != (UICR_REGOUT0_VOUT_3V0 << UICR_REGOUT0_VOUT_Pos))
        {
            NRF_NVMC->CONFIG = NRF_NVMC_MODE_WRITE;

            while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
            {
            }

            NRF_UICR->REGOUT0 = (NRF_UICR->REGOUT0 & ~((uint32_t)UICR_REGOUT0_VOUT_Msk)) |
                                (UICR_REGOUT0_VOUT_3V0 << UICR_REGOUT0_VOUT_Pos);

            NRF_NVMC->CONFIG = NRF_NVMC_MODE_READONLY;

            while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
            {
            }

            // system reset is needed to update UICR registers
            NVIC_SystemReset();
        }

        sd_softdevice_vector_table_base_set(FLASH_START_ADDR);

        BOARD_ERROR_CHECK(detail::flash::init(), true);
    }

    void halDeinit()
    {
    }

    void clocks()
    {
        nrfx_clock_init([](nrfx_clock_evt_type_t event)
                        {
                            switch (event)
                            {
                            case NRFX_CLOCK_EVT_HFCLK_STARTED:
                            {
                            }
                            break;

                            case NRFX_CLOCK_EVT_LFCLK_STARTED:
                            {
                            }
                            break;

                            default:
                                break;
                            }
                        });

        nrfx_clock_hfclk_start();
        nrfx_clock_lfclk_start();

        while (!nrfx_clock_hfclk_is_running())
        {
        }

        while (!nrfx_clock_lfclk_is_running())
        {
        }
    }

#ifdef ADC_SUPPORTED
    void adc()
    {
        nrf_saadc_event_clear(NRF_SAADC, NRF_SAADC_EVENT_STARTED);
        nrf_saadc_event_clear(NRF_SAADC, NRF_SAADC_EVENT_STOPPED);
        nrf_saadc_event_clear(NRF_SAADC, NRF_SAADC_EVENT_END);
        nrf_saadc_int_set(NRF_SAADC, 0);
        NRFX_IRQ_ENABLE(SAADC_IRQn);
        NRFX_IRQ_PRIORITY_SET(SAADC_IRQn, IRQ_PRIORITY_ADC);

        nrf_saadc_channel_config_t channelConfig = {
            .resistor_p = NRF_SAADC_RESISTOR_DISABLED,
            .resistor_n = NRF_SAADC_RESISTOR_DISABLED,
            .gain       = NRF_SAADC_GAIN1_5,
            .reference  = NRF_SAADC_REFERENCE_INTERNAL,
            .acq_time   = NRF_SAADC_ACQTIME_10US,
            .mode       = NRF_SAADC_MODE_SINGLE_ENDED,
            .burst      = NRF_SAADC_BURST_ENABLED,
        };

        for (size_t i = 0; i < MAX_ADC_CHANNELS; i++)
        {
            nrf_saadc_channel_init(NRF_SAADC, i, &channelConfig);
        }

        nrf_saadc_buffer_init(NRF_SAADC, &_adcValue, 1);

        // calibrate adc
        nrf_saadc_enable(NRF_SAADC);
        nrf_saadc_task_trigger(NRF_SAADC, NRF_SAADC_TASK_CALIBRATEOFFSET);

        while (!nrf_saadc_event_check(NRF_SAADC, NRF_SAADC_EVENT_CALIBRATEDONE))
        {
        }

        nrf_saadc_event_clear(NRF_SAADC, NRF_SAADC_EVENT_CALIBRATEDONE);

        // set first channel
        core::adc::setChannel(map::adcChannel(0));
        core::adc::startConversion();
    }
#endif

#ifdef USB_SUPPORTED
    void usb()
    {
        NVIC_SetPriority(USBD_IRQn, IRQ_PRIORITY_USB);

        // USB power may already be ready at this time in which case no event is generated.
        // We need to invoke the handler based on the initial status.
        uint32_t usbReg = 0;

#ifdef SOFTDEVICE_PRESENT
        uint8_t sd_en = false;
        sd_softdevice_is_enabled(&sd_en);

        if (sd_en)
        {
            sd_power_usbdetected_enable(true);
            sd_power_usbpwrrdy_enable(true);
            sd_power_usbremoved_enable(true);

            sd_power_usbregstatus_get(&usbReg);
        }
        else
#endif
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
        detail::registerUpdateHook(&tud_task);
    }
#endif
}    // namespace Board::detail::setup