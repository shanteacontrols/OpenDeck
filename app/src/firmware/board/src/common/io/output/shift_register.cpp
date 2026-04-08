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

#ifdef PROJECT_TARGET_SUPPORT_DIGITAL_OUTPUTS
#ifdef PROJECT_TARGET_DRIVER_DIGITAL_OUTPUT_SHIFT_REGISTER

#include "board/board.h"
#include "helpers.h"
#include "internal.h"
#include <target.h>

#include "core/util/util.h"

using namespace board::io::digital_out;
using namespace board::detail;
using namespace board::detail::io::digital_out;

namespace
{
    uint8_t          pwmCounter;
    volatile uint8_t ledState[(PROJECT_TARGET_MAX_NR_OF_DIGITAL_OUTPUTS / 8) + 1][static_cast<uint8_t>(ledBrightness_t::B100)];
}    // namespace

namespace board::detail::io::digital_out
{
    void init()
    {
        CORE_MCU_IO_INIT(PIN_PORT_SR_OUT_DATA,
                         PIN_INDEX_SR_OUT_DATA,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);

        CORE_MCU_IO_INIT(PIN_PORT_SR_OUT_CLK,
                         PIN_INDEX_SR_OUT_CLK,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);

        CORE_MCU_IO_INIT(PIN_PORT_SR_OUT_LATCH,
                         PIN_INDEX_SR_OUT_LATCH,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);

#ifdef PIN_PORT_SR_OUT_OE
        CORE_MCU_IO_INIT(PIN_PORT_SR_OUT_OE,
                         PIN_INDEX_SR_OUT_OE,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);
#endif

        // this will init all outputs to their default state (off)
        update();

        pwmCounter = 0;
    }

    void update()
    {
        CORE_MCU_IO_SET_LOW(PIN_PORT_SR_OUT_LATCH, PIN_INDEX_SR_OUT_LATCH);

        for (uint8_t shiftRegister = 0; shiftRegister < PROJECT_TARGET_NR_OF_OUT_SR; shiftRegister++)
        {
            for (uint8_t output = 0; output < 8; output++)
            {
                size_t  index      = output + shiftRegister * 8;
                uint8_t arrayIndex = index / 8;
                uint8_t bit        = index - 8 * arrayIndex;

                core::util::BIT_READ(ledState[arrayIndex][pwmCounter], bit)
                    ? EXT_LED_ON(PIN_PORT_SR_OUT_DATA, PIN_INDEX_SR_OUT_DATA)
                    : EXT_LED_OFF(PIN_PORT_SR_OUT_DATA, PIN_INDEX_SR_OUT_DATA);

                CORE_MCU_IO_SET_LOW(PIN_PORT_SR_OUT_CLK, PIN_INDEX_SR_OUT_CLK);
                detail::io::spiWait();
                CORE_MCU_IO_SET_HIGH(PIN_PORT_SR_OUT_CLK, PIN_INDEX_SR_OUT_CLK);
            }
        }

        CORE_MCU_IO_SET_HIGH(PIN_PORT_SR_OUT_LATCH, PIN_INDEX_SR_OUT_LATCH);

        if (++pwmCounter >= static_cast<uint8_t>(ledBrightness_t::B100))
        {
            pwmCounter = 0;
        }
    }
}    // namespace board::detail::io::digital_out

namespace board::io::digital_out
{
    void writeLedState(size_t index, ledBrightness_t ledBrightness)
    {
        if (index >= PROJECT_TARGET_MAX_NR_OF_DIGITAL_OUTPUTS)
        {
            return;
        }

        index = map::LED_INDEX(index);

        CORE_MCU_ATOMIC_SECTION
        {
            for (uint8_t i = 0; i < static_cast<int>(ledBrightness_t::B100); i++)
            {
                uint8_t arrayIndex = index / 8;
                uint8_t bit        = index - 8 * arrayIndex;

                core::util::BIT_WRITE(ledState[arrayIndex][i], bit, i < static_cast<int>(ledBrightness) ? 1 : 0);
            }
        }
    }

    size_t rgbFromOutput(size_t index)
    {
        uint8_t result = index / 3;

        if (result >= PROJECT_TARGET_NR_OF_RGB_LEDS)
        {
            return PROJECT_TARGET_NR_OF_RGB_LEDS - 1;
        }

        return result;
    }

    size_t rgbComponentFromRgb(size_t index, rgbComponent_t component)
    {
        return index * 3 + static_cast<uint8_t>(component);
    }
}    // namespace board::io::digital_out

#endif
#endif