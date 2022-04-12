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

#ifdef NUMBER_OF_MUX

#include "board/Board.h"
#include "board/Internal.h"
#include <Target.h>

namespace Board::detail::IO
{
    void dischargeMux()
    {
        // discharge the voltage present on common mux pins to avoid channel crosstalk
        for (int i = 0; i < MAX_ADC_CHANNELS; i++)
        {
            core::mcu::io::pin_t pin = Board::detail::map::adcPin(i);

            CORE_MCU_IO_INIT(CORE_MCU_IO_PIN_PORT(pin), CORE_MCU_IO_PIN_INDEX(pin), core::mcu::io::pinMode_t::OUTPUT);
            CORE_MCU_IO_SET_LOW(CORE_MCU_IO_PIN_PORT(pin), CORE_MCU_IO_PIN_INDEX(pin));
        }
    }

    void restoreMux(uint8_t muxIndex)
    {
        core::mcu::io::pin_t pin = detail::map::adcPin(muxIndex);
        CORE_MCU_IO_INIT(CORE_MCU_IO_PIN_PORT(pin), CORE_MCU_IO_PIN_INDEX(pin), core::mcu::io::pinMode_t::INPUT);
    }
}    // namespace Board::detail::IO

#endif