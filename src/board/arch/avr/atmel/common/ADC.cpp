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

#ifdef FW_APP
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
        core::mcu::adc::conf_t adcConfiguration;

        adcConfiguration.prescaler = core::mcu::adc::prescaler_t::P128;

#ifdef ADC_EXT_REF
        adcConfiguration.vref = core::mcu::adc::vRef_t::AREF;
#else
        adcConfiguration.vref = core::mcu::adc::vRef_t::AVCC;
#endif

        for (size_t i = 0; i < MAX_ADC_CHANNELS; i++)
        {
            core::mcu::adc::disconnectDigitalIn(Board::detail::map::adcChannel(i));
        }

        core::mcu::adc::init(adcConfiguration);

        for (uint8_t i = 0; i < 3; i++)
        {
            // few dummy reads to init ADC
            core::mcu::adc::read(Board::detail::map::adcChannel(0));
        }

        core::mcu::adc::startItConversion();
    }
}    // namespace Board::detail::IO::analog::MCU

#endif
#endif