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

#ifdef DIGITAL_INPUTS_SUPPORTED
#ifdef DIGITAL_INPUT_DRIVER_SHIFT_REGISTER

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "board/Board.h"
#include "board/Internal.h"
#include "core/src/util/Util.h"
#include "core/src/util/RingBuffer.h"
#include <Target.h>

using namespace Board::IO::digitalIn;
using namespace Board::detail;
using namespace Board::detail::IO::digitalIn;

namespace
{
    volatile readings_t _digitalInBuffer[NR_OF_DIGITAL_INPUTS];

    inline void storeDigitalIn()
    {
        CORE_MCU_IO_SET_LOW(PIN_PORT_SR_IN_CLK, PIN_INDEX_SR_IN_CLK);
        CORE_MCU_IO_SET_LOW(PIN_PORT_SR_IN_LATCH, PIN_INDEX_SR_IN_LATCH);
        IO::spiWait();

        CORE_MCU_IO_SET_HIGH(PIN_PORT_SR_IN_LATCH, PIN_INDEX_SR_IN_LATCH);

        for (uint8_t shiftRegister = 0; shiftRegister < NUMBER_OF_IN_SR; shiftRegister++)
        {
            for (uint8_t input = 0; input < 8; input++)
            {
                //  register shifts out MSB first
                size_t index = (shiftRegister * 8) + (7 - input);
                CORE_MCU_IO_SET_LOW(PIN_PORT_SR_IN_CLK, PIN_INDEX_SR_IN_CLK);
                IO::spiWait();

                _digitalInBuffer[index].readings <<= 1;
                _digitalInBuffer[index].readings |= !CORE_MCU_IO_READ(PIN_PORT_SR_IN_DATA, PIN_INDEX_SR_IN_DATA);

                if (++_digitalInBuffer[index].count > MAX_READING_COUNT)
                {
                    _digitalInBuffer[index].count = MAX_READING_COUNT;
                }

                CORE_MCU_IO_SET_HIGH(PIN_PORT_SR_IN_CLK, PIN_INDEX_SR_IN_CLK);
            }
        }
    }
}    // namespace

namespace Board::detail::IO::digitalIn
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
}    // namespace Board::detail::IO::digitalIn

namespace Board::IO::digitalIn
{
    bool state(size_t index, readings_t& readings)
    {
        if (index >= NR_OF_DIGITAL_INPUTS)
        {
            return false;
        }

        index = detail::map::buttonIndex(index);

        CORE_MCU_ATOMIC_SECTION
        {
            readings.count                = _digitalInBuffer[index].count;
            readings.readings             = _digitalInBuffer[index].readings;
            _digitalInBuffer[index].count = 0;
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
}    // namespace Board::IO::digitalIn

#include "Common.cpp.include"

#endif
#endif