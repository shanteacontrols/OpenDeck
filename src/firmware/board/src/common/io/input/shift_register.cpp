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

#ifdef PROJECT_TARGET_SUPPORT_DIGITAL_INPUTS
#ifdef PROJECT_TARGET_DRIVER_DIGITAL_INPUT_SHIFT_REGISTER

#include "board/board.h"
#include "internal.h"
#include <target.h>

#include "core/util/util.h"
#include "core/util/ring_buffer.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

using namespace board::io::digital_in;
using namespace board::detail;
using namespace board::detail::io::digital_in;

namespace
{
    volatile Readings digitalInBuffer[PROJECT_TARGET_MAX_NR_OF_DIGITAL_INPUTS];

    inline void storeDigitalIn()
    {
        CORE_MCU_IO_SET_LOW(PIN_PORT_SR_IN_CLK, PIN_INDEX_SR_IN_CLK);
        CORE_MCU_IO_SET_LOW(PIN_PORT_SR_IN_LATCH, PIN_INDEX_SR_IN_LATCH);
        io::spiWait();

        CORE_MCU_IO_SET_HIGH(PIN_PORT_SR_IN_LATCH, PIN_INDEX_SR_IN_LATCH);

        for (uint8_t shiftRegister = 0; shiftRegister < PROJECT_TARGET_NR_OF_IN_SR; shiftRegister++)
        {
            for (uint8_t input = 0; input < 8; input++)
            {
                //  register shifts out MSB first
                size_t index = (shiftRegister * 8) + (7 - input);
                CORE_MCU_IO_SET_LOW(PIN_PORT_SR_IN_CLK, PIN_INDEX_SR_IN_CLK);
                io::spiWait();

                digitalInBuffer[index].readings <<= 1;
                digitalInBuffer[index].readings |= !CORE_MCU_IO_READ(PIN_PORT_SR_IN_DATA, PIN_INDEX_SR_IN_DATA);

                if (++digitalInBuffer[index].count > MAX_READING_COUNT)
                {
                    digitalInBuffer[index].count = MAX_READING_COUNT;
                }

                CORE_MCU_IO_SET_HIGH(PIN_PORT_SR_IN_CLK, PIN_INDEX_SR_IN_CLK);
            }
        }
    }
}    // namespace

namespace board::detail::io::digital_in
{
    void init()
    {
        CORE_MCU_IO_INIT(PIN_PORT_SR_IN_DATA,
                         PIN_INDEX_SR_IN_DATA,
                         core::mcu::io::pinMode_t::INPUT);

        CORE_MCU_IO_INIT(PIN_PORT_SR_IN_CLK,
                         PIN_INDEX_SR_IN_CLK,
                         core::mcu::io::pinMode_t::OUTPUT_PP);

        CORE_MCU_IO_INIT(PIN_PORT_SR_IN_LATCH,
                         PIN_INDEX_SR_IN_LATCH,
                         core::mcu::io::pinMode_t::OUTPUT_PP);

        CORE_MCU_IO_SET_LOW(PIN_PORT_SR_IN_CLK, PIN_INDEX_SR_IN_CLK);
        CORE_MCU_IO_SET_HIGH(PIN_PORT_SR_IN_LATCH, PIN_INDEX_SR_IN_LATCH);
    }
}    // namespace board::detail::io::digital_in

namespace board::io::digital_in
{
    bool state(size_t index, Readings& readings)
    {
        if (index >= PROJECT_TARGET_MAX_NR_OF_DIGITAL_INPUTS)
        {
            return false;
        }

        index = detail::map::BUTTON_INDEX(index);

        CORE_MCU_ATOMIC_SECTION
        {
            readings.count               = digitalInBuffer[index].count;
            readings.readings            = digitalInBuffer[index].readings;
            digitalInBuffer[index].count = 0;
        }

        return readings.count > 0;
    }

    size_t encoderFromInput(size_t index)
    {
        return index / 2;
    }

    size_t encoderComponentFromEncoder(size_t index, encoderComponent_t component)
    {
        index *= 2;

        if (component == encoderComponent_t::A)
        {
            return index;
        }

        return index + 1;
    }
}    // namespace board::io::digital_in

#include "common.cpp.include"

#endif
#endif