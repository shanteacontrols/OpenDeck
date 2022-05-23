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
#ifdef ADC_SUPPORTED

#include "board/Board.h"
#include "board/Internal.h"
#include <Target.h>

void core::mcu::isr::adc(uint16_t value)
{
    Board::detail::IO::analog::isr(value);
}

namespace Board::detail::IO::analog::MCU
{
    void init()
    {
        ADC_HandleTypeDef      adcHandler = {};
        ADC_ChannelConfTypeDef sConfig    = {};

        switch (CORE_CPU_FREQ_MHZ)
        {
        case 84:
        {
            adcHandler.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
        }
        break;

        case 168:
        {
            adcHandler.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
        }
        break;

        default:
            return;    // unsupported
        }

        adcHandler.Instance                   = ADC1;
        adcHandler.Init.Resolution            = ADC_RESOLUTION_12B;
        adcHandler.Init.ScanConvMode          = DISABLE;
        adcHandler.Init.ContinuousConvMode    = DISABLE;
        adcHandler.Init.DiscontinuousConvMode = DISABLE;
        adcHandler.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
        adcHandler.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
        adcHandler.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
        adcHandler.Init.NbrOfConversion       = 1;
        adcHandler.Init.DMAContinuousRequests = DISABLE;
        adcHandler.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
        HAL_ADC_Init(&adcHandler);

        for (size_t i = 0; i < MAX_ADC_CHANNELS; i++)
        {
            sConfig.Channel      = map::adcChannel(i);
            sConfig.Rank         = 1;
            sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
            HAL_ADC_ConfigChannel(&adcHandler, &sConfig);
        }

        // set first channel
        core::mcu::adc::setChannel(map::adcChannel(0));

        HAL_ADC_Start_IT(&adcHandler);
    }
}    // namespace Board::detail::IO::analog::MCU

#endif
#endif