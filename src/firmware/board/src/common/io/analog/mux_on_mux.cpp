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
#ifdef PROJECT_TARGET_DRIVER_ANALOG_INPUT_MUXONMUX

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
    uint8_t           activeMux;
    uint8_t           activeMuxInput;
    volatile uint16_t sample;
    volatile uint8_t  sampleCounter;

    /// Configures one of 16 inputs/outputs on 4067 multiplexer.
    inline void setMuxInput()
    {
        CORE_MCU_IO_SET_STATE(PIN_PORT_MUX_NODE_S0, PIN_INDEX_MUX_NODE_S0, core::util::BIT_READ(activeMuxInput, 0));
        CORE_MCU_IO_SET_STATE(PIN_PORT_MUX_NODE_S1, PIN_INDEX_MUX_NODE_S1, core::util::BIT_READ(activeMuxInput, 1));
        CORE_MCU_IO_SET_STATE(PIN_PORT_MUX_NODE_S2, PIN_INDEX_MUX_NODE_S2, core::util::BIT_READ(activeMuxInput, 2));
        CORE_MCU_IO_SET_STATE(PIN_PORT_MUX_NODE_S3, PIN_INDEX_MUX_NODE_S3, core::util::BIT_READ(activeMuxInput, 3));
    }

    inline void setMux()
    {
        CORE_MCU_IO_SET_STATE(PIN_PORT_MUX_CTRL_S0, PIN_INDEX_MUX_CTRL_S0, core::util::BIT_READ(activeMux, 0));
        CORE_MCU_IO_SET_STATE(PIN_PORT_MUX_CTRL_S1, PIN_INDEX_MUX_CTRL_S1, core::util::BIT_READ(activeMux, 1));
#ifdef PIN_PORT_MUX_CTRL_S2
        CORE_MCU_IO_SET_STATE(PIN_PORT_MUX_CTRL_S2, PIN_INDEX_MUX_CTRL_S2, core::util::BIT_READ(activeMux, 2));
#endif
#ifdef PIN_PORT_MUX_CTRL_S3
        CORE_MCU_IO_SET_STATE(PIN_PORT_MUX_CTRL_S3, PIN_INDEX_MUX_CTRL_S3, core::util::BIT_READ(activeMux, 3));
#endif
    }
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

        auto pin = map::ADC_PIN(0);
        core::mcu::adc::initPin(pin);

        CORE_MCU_IO_INIT(PIN_PORT_MUX_CTRL_S0,
                         PIN_INDEX_MUX_CTRL_S0,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);

        CORE_MCU_IO_INIT(PIN_PORT_MUX_CTRL_S1,
                         PIN_INDEX_MUX_CTRL_S1,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);

#ifdef PIN_PORT_MUX_CTRL_S2
        CORE_MCU_IO_INIT(PIN_PORT_MUX_CTRL_S2,
                         PIN_INDEX_MUX_CTRL_S2,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);
#endif

#ifdef PIN_PORT_MUX_CTRL_S3
        CORE_MCU_IO_INIT(PIN_PORT_MUX_CTRL_S3,
                         PIN_INDEX_MUX_CTRL_S3,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);
#endif

        CORE_MCU_IO_INIT(PIN_PORT_MUX_NODE_S0,
                         PIN_INDEX_MUX_NODE_S0,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);

        CORE_MCU_IO_INIT(PIN_PORT_MUX_NODE_S1,
                         PIN_INDEX_MUX_NODE_S1,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);

        CORE_MCU_IO_INIT(PIN_PORT_MUX_NODE_S2,
                         PIN_INDEX_MUX_NODE_S2,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);

        CORE_MCU_IO_INIT(PIN_PORT_MUX_NODE_S3,
                         PIN_INDEX_MUX_NODE_S3,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);

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
                activeMuxInput++;

                bool switchMux = (activeMuxInput == PROJECT_TARGET_NR_OF_MUX_INPUTS);

                if (switchMux)
                {
                    activeMuxInput = 0;
                    activeMux++;

                    if (activeMux == PROJECT_TARGET_NR_OF_MUX)
                    {
                        activeMux   = 0;
                        analogIndex = 0;
                    }

                    // switch to next mux once all mux inputs are read
                    setMux();
                }

                // always switch to next read pin
                setMuxInput();
            }
        }

        core::mcu::adc::startItConversion();
    }
}    // namespace board::detail::io::analog

#include "common.cpp.include"

#endif
#endif