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

#ifdef PROJECT_TARGET_SUPPORT_ADC
#ifdef PROJECT_TARGET_DRIVER_ANALOG_INPUT_NATIVE

#include "board/board.h"
#include "internal.h"
#include <target.h>

#include "core/util/util.h"

using namespace board::io::analog;
using namespace board::detail;
using namespace board::detail::io::analog;

static_assert(PROJECT_MCU_ADC_SAMPLES > 0, "At least 1 ADC sample required");

namespace
{
    constexpr size_t  ANALOG_IN_BUFFER_SIZE = PROJECT_TARGET_MAX_NR_OF_ANALOG_INPUTS;
    uint8_t           analogIndex;
    volatile uint16_t analogBuffer[ANALOG_IN_BUFFER_SIZE];
    volatile uint16_t sample;
    volatile uint8_t  sampleCounter;
}    // namespace

namespace board::detail::io::analog
{
    void init()
    {
        core::mcu::adc::conf_t adcConfiguration;

        adcConfiguration.prescaler = PROJECT_MCU_ADC_PRESCALER;
        adcConfiguration.voltage   = PROJECT_TARGET_ADC_INPUT_VOLTAGE;

#ifdef PROJECT_TARGET_ADC_EXT_REF
        adcConfiguration.externalRef = true;
#else
        adcConfiguration.externalRef = false;
#endif

        core::mcu::adc::init(adcConfiguration);

        for (size_t i = 0; i < PROJECT_TARGET_NR_OF_ADC_CHANNELS; i++)
        {
            auto pin = map::ADC_PIN(i);
            core::mcu::adc::initPin(pin);
        }

        for (uint8_t i = 0; i < 3; i++)
        {
            // few dummy reads to init ADC
            core::mcu::adc::read(map::ADC_PIN(0));
        }

        core::mcu::adc::setActivePin(map::ADC_PIN(0));
        core::mcu::adc::enableIt(board::detail::io::analog::ISR_PRIORITY);
        core::mcu::adc::startItConversion();
    }

    void isr(uint16_t adcValue)
    {
        if (adcValue <= CORE_MCU_ADC_MAX_VALUE)
        {
            // always ignore first sample
            if (sampleCounter)
            {
                sample += adcValue;
            }

            if (++sampleCounter == (PROJECT_MCU_ADC_SAMPLES + 1))
            {
                sample /= PROJECT_MCU_ADC_SAMPLES;
                analogBuffer[analogIndex] = sample;
                analogBuffer[analogIndex] |= ADC_NEW_READING_FLAG;
                sample        = 0;
                sampleCounter = 0;
                analogIndex++;

                if (analogIndex == PROJECT_TARGET_MAX_NR_OF_ANALOG_INPUTS)
                {
                    analogIndex = 0;
                }

                // always switch to next read pin
                core::mcu::adc::setActivePin(map::ADC_PIN(analogIndex));
            }
        }

        core::mcu::adc::startItConversion();
    }
}    // namespace board::detail::io::analog

#include "common.cpp.include"

#endif
#endif