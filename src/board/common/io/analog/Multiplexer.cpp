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
#ifdef ANALOG_INPUT_DRIVER_MULTIPLEXER

#include "core/src/util/Util.h"
#include "board/Board.h"
#include "board/Internal.h"
#include <Target.h>

using namespace Board::IO::analog;
using namespace Board::detail;
using namespace Board::detail::IO::analog;

namespace
{
    constexpr size_t ANALOG_IN_BUFFER_SIZE = (NUMBER_OF_MUX_INPUTS * NUMBER_OF_MUX);

    uint8_t           _analogIndex;
    volatile uint16_t _analogBuffer[ANALOG_IN_BUFFER_SIZE];
    uint8_t           _activeMux;
    uint8_t           _activeMuxInput;

    /// Configures one of 16 inputs/outputs on 4067 multiplexer.
    inline void setMuxInput()
    {
        CORE_MCU_IO_SET_STATE(PIN_PORT_MUX_S0, PIN_INDEX_MUX_S0, core::util::BIT_READ(_activeMuxInput, 0));
        CORE_MCU_IO_SET_STATE(PIN_PORT_MUX_S1, PIN_INDEX_MUX_S1, core::util::BIT_READ(_activeMuxInput, 1));
        CORE_MCU_IO_SET_STATE(PIN_PORT_MUX_S2, PIN_INDEX_MUX_S2, core::util::BIT_READ(_activeMuxInput, 2));
#ifdef PIN_PORT_MUX_S3
        CORE_MCU_IO_SET_STATE(PIN_PORT_MUX_S3, PIN_INDEX_MUX_S3, core::util::BIT_READ(_activeMuxInput, 3));
#endif
    }

    void dischargeMux()
    {
        // discharge the voltage present on common mux pins to avoid channel crosstalk
        for (size_t i = 0; i < MAX_ADC_CHANNELS; i++)
        {
            auto pin = map::adcPin(i);

            CORE_MCU_IO_INIT(CORE_MCU_IO_PIN_PORT(pin), CORE_MCU_IO_PIN_INDEX(pin), core::mcu::io::pinMode_t::OUTPUT_PP);
            CORE_MCU_IO_SET_LOW(CORE_MCU_IO_PIN_PORT(pin), CORE_MCU_IO_PIN_INDEX(pin));
        }
    }

    void restoreMux(uint8_t muxIndex)
    {
        auto pin = map::adcPin(muxIndex);
        CORE_MCU_IO_INIT(CORE_MCU_IO_PIN_PORT(pin), CORE_MCU_IO_PIN_INDEX(pin), core::mcu::io::pinMode_t::ANALOG);
    }
}    // namespace

namespace Board::detail::IO::analog
{
    void init()
    {
        MCU::init();

        for (size_t i = 0; i < MAX_ADC_CHANNELS; i++)
        {
            auto pin = map::adcPin(i);
            CORE_MCU_IO_INIT(CORE_MCU_IO_PIN_PORT(pin), CORE_MCU_IO_PIN_INDEX(pin), core::mcu::io::pinMode_t::ANALOG);
            CORE_MCU_IO_SET_LOW(CORE_MCU_IO_PIN_PORT(pin), CORE_MCU_IO_PIN_INDEX(pin));
        }

        CORE_MCU_IO_INIT(PIN_PORT_MUX_S0,
                         PIN_INDEX_MUX_S0,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);

        CORE_MCU_IO_INIT(PIN_PORT_MUX_S1,
                         PIN_INDEX_MUX_S1,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);

        CORE_MCU_IO_INIT(PIN_PORT_MUX_S2,
                         PIN_INDEX_MUX_S2,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);
#ifdef PIN_PORT_MUX_S3
        CORE_MCU_IO_INIT(PIN_PORT_MUX_S3,
                         PIN_INDEX_MUX_S3,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);
#endif
    }

    void isr(uint16_t adcValue)
    {
        static bool firstReading = false;
        firstReading             = !firstReading;

        if (!firstReading && (adcValue <= core::mcu::adc::MAX))
        {
            dischargeMux();

            _analogBuffer[_analogIndex] = adcValue | ADC_NEW_READING_FLAG;
            _analogIndex++;
            _activeMuxInput++;

            bool switchMux = (_activeMuxInput == NUMBER_OF_MUX_INPUTS);

            if (switchMux)
            {
                _activeMuxInput = 0;
                _activeMux++;

                if (_activeMux == NUMBER_OF_MUX)
                {
                    _activeMux   = 0;
                    _analogIndex = 0;
                }

                // switch to next mux once all mux inputs are read
                core::mcu::adc::setChannel(map::adcChannel(_activeMux));
            }

            // always switch to next read pin
            setMuxInput();
        }

        restoreMux(_activeMux);

        core::mcu::adc::startItConversion();
    }
}    // namespace Board::detail::IO::analog

#include "Common.cpp.include"

#endif
#endif