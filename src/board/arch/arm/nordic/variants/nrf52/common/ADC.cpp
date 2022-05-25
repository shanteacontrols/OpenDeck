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

#if defined(FW_APP)
// not needed in bootloader
#ifdef FW_APP
#ifdef ADC_SUPPORTED

#include "board/Board.h"
#include "board/Internal.h"
#include "nrfx_saadc.h"
#include <Target.h>

namespace
{
    // nrf requires setting value from ram as an buffer in which adc value will be stored
    nrf_saadc_value_t _adcValue;

    inline uint32_t scaleValue(uint32_t value)
    {
        if (value > core::mcu::adc::MAX)
        {
            // invalid value
            return 0xFFFFFFFF;
        }

#ifdef ADC_INPUT_V_33
        // if 3.3V input analog voltage is used, ADC requires gain setting of 3.6V (nothing between 3.0V and 3.6V)
        // calculate maximum range for 3.3V and scale (difference is ~9%)
        static constexpr uint32_t ADC_MAX_RANGE_V33 =
            core::mcu::adc::MAX - static_cast<uint32_t>(static_cast<float>(core::mcu::adc::MAX) * 0.09);

        return core::util::MAP_RANGE(value,
                                     static_cast<uint32_t>(0),
                                     ADC_MAX_RANGE_V33,
                                     static_cast<uint32_t>(0),
                                     core::mcu::adc::MAX);
#else
        return value;
#endif
    }
}    // namespace

void core::mcu::isr::adc(uint32_t value)
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
        auto scaled = scaleValue(value);
        Board::detail::IO::analog::isr(scaled);
    }

    if (nrf_saadc_event_check(NRF_SAADC, NRF_SAADC_EVENT_CALIBRATEDONE))
    {
        nrf_saadc_event_clear(NRF_SAADC, NRF_SAADC_EVENT_CALIBRATEDONE);
    }
}

namespace Board::detail::IO::analog::MCU
{
    void init()
    {
        nrf_saadc_channel_config_t channelConfig = {
            .resistor_p = NRF_SAADC_RESISTOR_DISABLED,
            .resistor_n = NRF_SAADC_RESISTOR_DISABLED,
#ifdef ADC_INPUT_V_33
            .gain = NRF_SAADC_GAIN1_6,
#else
            .gain = NRF_SAADC_GAIN1_5,
#endif
            .reference = NRF_SAADC_REFERENCE_INTERNAL,
            .acq_time  = NRF_SAADC_ACQTIME_10US,
            .mode      = NRF_SAADC_MODE_SINGLE_ENDED,
            .burst     = NRF_SAADC_BURST_ENABLED,
        };

        for (size_t i = 0; i < MAX_ADC_CHANNELS; i++)
        {
            nrf_saadc_channel_init(NRF_SAADC, i, &channelConfig);
        }

        nrf_saadc_buffer_init(NRF_SAADC, &_adcValue, 1);

        nrf_saadc_enable(NRF_SAADC);
        nrf_saadc_task_trigger(NRF_SAADC, NRF_SAADC_TASK_CALIBRATEOFFSET);

        while (!nrf_saadc_event_check(NRF_SAADC, NRF_SAADC_EVENT_CALIBRATEDONE))
        {
        }

        nrf_saadc_event_clear(NRF_SAADC, NRF_SAADC_EVENT_CALIBRATEDONE);

        nrf_saadc_resolution_set(NRF_SAADC, NRF_SAADC_RESOLUTION_12BIT);
        nrf_saadc_oversample_set(NRF_SAADC, NRF_SAADC_OVERSAMPLE_16X);
        nrf_saadc_int_set(NRF_SAADC, NRF_SAADC_INT_STARTED | NRF_SAADC_INT_STOPPED | NRF_SAADC_INT_END);

        NRFX_IRQ_PRIORITY_SET(SAADC_IRQn, IRQ_PRIORITY_ADC);
        NRFX_IRQ_ENABLE(SAADC_IRQn);

        // set first channel
        core::mcu::adc::setChannel(map::adcChannel(0));
        core::mcu::adc::startItConversion();
    }
}    // namespace Board::detail::IO::analog::MCU

#endif
#endif
#endif
