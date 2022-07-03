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

#ifdef ADC_SUPPORTED
#ifdef ANALOG_INPUT_DRIVER_NATIVE

#include "core/src/util/Util.h"
#include "board/Board.h"
#include "board/Internal.h"
#include <Target.h>

using namespace Board::IO::analog;
using namespace Board::detail;
using namespace Board::detail::IO::analog;

static_assert(ADC_SAMPLES > 0, "At least 1 ADC sample required");

namespace
{
    constexpr size_t ANALOG_IN_BUFFER_SIZE = NR_OF_ANALOG_INPUTS;

    uint8_t           _analogIndex;
    volatile uint16_t _analogBuffer[ANALOG_IN_BUFFER_SIZE];
    volatile uint16_t _sample;
    volatile uint8_t  _sampleCounter;
}    // namespace

namespace Board::detail::IO::analog
{
    void init()
    {
        core::mcu::adc::conf_t adcConfiguration;

        adcConfiguration.prescaler = ADC_PRESCALER;
        adcConfiguration.voltage   = ADC_INPUT_VOLTAGE;

#ifdef ADC_EXT_REF
        adcConfiguration.externalRef = true;
#else
        adcConfiguration.externalRef = false;
#endif

        core::mcu::adc::init(adcConfiguration);

        for (size_t i = 0; i < NR_OF_ADC_CHANNELS; i++)
        {
            auto pin = map::adcPin(i);
            core::mcu::adc::initPin(pin);
        }

        for (uint8_t i = 0; i < 3; i++)
        {
            // few dummy reads to init ADC
            core::mcu::adc::read(map::adcPin(0));
        }

        core::mcu::adc::setActivePin(map::adcPin(0));
        core::mcu::adc::enableIt(Board::detail::IO::analog::ISR_PRIORITY);
        core::mcu::adc::startItConversion();
    }

    void isr(uint16_t adcValue)
    {
        if (adcValue <= CORE_MCU_ADC_MAX_VALUE)
        {
            // always ignore first sample
            if (_sampleCounter)
            {
                _sample += adcValue;
            }

            if (++_sampleCounter == (ADC_SAMPLES + 1))
            {
                _sample /= ADC_SAMPLES;
                _analogBuffer[_analogIndex] = _sample;
                _analogBuffer[_analogIndex] |= ADC_NEW_READING_FLAG;
                _sample        = 0;
                _sampleCounter = 0;
                _analogIndex++;

                if (_analogIndex == NR_OF_ANALOG_INPUTS)
                {
                    _analogIndex = 0;
                }

                // always switch to next read pin
                core::mcu::adc::setActivePin(map::adcPin(_analogIndex));
            }
        }

        core::mcu::adc::startItConversion();
    }
}    // namespace Board::detail::IO::analog

#include "Common.cpp.include"

#endif
#endif