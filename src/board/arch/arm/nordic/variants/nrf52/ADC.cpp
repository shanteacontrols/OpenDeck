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
#include "nrfx_saadc.h"
#include <Target.h>

#if defined(FW_APP)
// not needed in bootloader
#ifdef FW_APP
#ifdef ADC_SUPPORTED

extern "C" void SAADC_IRQHandler(void)
{
    if (nrf_saadc_event_check(NRF_SAADC, NRF_SAADC_EVENT_STARTED))
    {
        nrf_saadc_event_clear(NRF_SAADC, NRF_SAADC_EVENT_STARTED);
        nrf_saadc_task_trigger(NRF_SAADC, NRF_SAADC_TASK_SAMPLE);
    }

    if (nrf_saadc_event_check(NRF_SAADC, NRF_SAADC_EVENT_STOPPED))
    {
        nrf_saadc_event_clear(NRF_SAADC, NRF_SAADC_EVENT_STOPPED);
    }

    if (nrf_saadc_event_check(NRF_SAADC, NRF_SAADC_EVENT_END))
    {
        nrf_saadc_event_clear(NRF_SAADC, NRF_SAADC_EVENT_END);
        Board::detail::isrHandling::adc(*reinterpret_cast<uint16_t*>(NRF_SAADC->RESULT.PTR));
    }

    if (nrf_saadc_event_check(NRF_SAADC, NRF_SAADC_EVENT_CALIBRATEDONE))
    {
        nrf_saadc_event_clear(NRF_SAADC, NRF_SAADC_EVENT_CALIBRATEDONE);
    }
}

namespace core::adc
{
    void startConversion()
    {
        nrf_saadc_task_trigger(NRF_SAADC, NRF_SAADC_TASK_START);
    }

    void setChannel(uint32_t adcChannel)
    {
        nrf_saadc_resolution_set(NRF_SAADC, NRF_SAADC_RESOLUTION_12BIT);
        nrf_saadc_oversample_set(NRF_SAADC, NRF_SAADC_OVERSAMPLE_16X);
        nrf_saadc_int_set(NRF_SAADC, NRF_SAADC_INT_STARTED | NRF_SAADC_INT_STOPPED | NRF_SAADC_INT_END);

        for (uint32_t ch_pos = 0; ch_pos < SAADC_CH_NUM; ch_pos++)
        {
            nrf_saadc_input_t pselp = NRF_SAADC_INPUT_DISABLED;

            if ((1 << adcChannel) & (1 << ch_pos))
            {
                pselp = static_cast<nrf_saadc_input_t>(ch_pos + 1);
            }

            nrf_saadc_burst_set(NRF_SAADC, ch_pos, NRF_SAADC_BURST_ENABLED);
            nrf_saadc_channel_input_set(NRF_SAADC, ch_pos, pselp, NRF_SAADC_INPUT_DISABLED);
        }
    }
}    // namespace core::adc

#endif
#endif
#endif
