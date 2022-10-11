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

#ifdef HW_SUPPORT_DIGITAL_INPUTS
#ifdef HW_DRIVER_DIGITAL_INPUT_MATRIX_SHIFT_REGISTER_ROWS

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
    volatile readings_t _digitalInBuffer[HW_MAX_NR_OF_DIGITAL_INPUTS];
    volatile uint8_t    _activeInColumn;

    inline void activateInputColumn()
    {
        CORE_MCU_IO_SET_STATE(PIN_PORT_DEC_BM_A0, PIN_INDEX_DEC_BM_A0, core::util::BIT_READ(_activeInColumn, 0));
        CORE_MCU_IO_SET_STATE(PIN_PORT_DEC_BM_A1, PIN_INDEX_DEC_BM_A1, core::util::BIT_READ(_activeInColumn, 1));
        CORE_MCU_IO_SET_STATE(PIN_PORT_DEC_BM_A2, PIN_INDEX_DEC_BM_A2, core::util::BIT_READ(_activeInColumn, 2));

        if (++_activeInColumn == HW_NR_OF_BUTTON_COLUMNS)
        {
            _activeInColumn = 0;
        }
    }

    inline void storeDigitalIn()
    {
        for (uint8_t column = 0; column < HW_NR_OF_BUTTON_COLUMNS; column++)
        {
            activateInputColumn();

            IO::spiWait();
            CORE_MCU_IO_SET_LOW(PIN_PORT_SR_IN_CLK, PIN_INDEX_SR_IN_CLK);
            CORE_MCU_IO_SET_LOW(PIN_PORT_SR_IN_LATCH, PIN_INDEX_SR_IN_LATCH);
            IO::spiWait();

            CORE_MCU_IO_SET_HIGH(PIN_PORT_SR_IN_LATCH, PIN_INDEX_SR_IN_LATCH);

            for (uint8_t row = 0; row < HW_NR_OF_BUTTON_ROWS; row++)
            {
                // this register shifts out MSB first
                size_t index = ((((HW_NR_OF_IN_SR * 8) - 1) - row) * HW_NR_OF_BUTTON_COLUMNS) + column;
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

        CORE_MCU_IO_INIT(PIN_PORT_DEC_BM_A0,
                         PIN_INDEX_DEC_BM_A0,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);

        CORE_MCU_IO_INIT(PIN_PORT_DEC_BM_A1,
                         PIN_INDEX_DEC_BM_A1,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);

        CORE_MCU_IO_INIT(PIN_PORT_DEC_BM_A2,
                         PIN_INDEX_DEC_BM_A2,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);

        CORE_MCU_IO_SET_LOW(PIN_PORT_DEC_BM_A0, PIN_INDEX_DEC_BM_A0);
        CORE_MCU_IO_SET_LOW(PIN_PORT_DEC_BM_A1, PIN_INDEX_DEC_BM_A1);
        CORE_MCU_IO_SET_LOW(PIN_PORT_DEC_BM_A2, PIN_INDEX_DEC_BM_A2);
    }
}    // namespace Board::detail::IO::digitalIn

namespace Board::IO::digitalIn
{
    bool state(size_t index, readings_t& readings)
    {
        if (index >= HW_MAX_NR_OF_DIGITAL_INPUTS)
        {
            return false;
        }

        index = map::BUTTON_INDEX(index);

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
        uint8_t row    = index / HW_NR_OF_BUTTON_COLUMNS;
        uint8_t column = index % HW_NR_OF_BUTTON_COLUMNS;

        if (row % 2)
        {
            row -= 1;    // uneven row, get info from previous (even) row
        }

        return (row * HW_NR_OF_BUTTON_COLUMNS) / 2 + column;
    }

    size_t encoderComponentFromEncoder(size_t index, encoderComponent_t component)
    {
        uint8_t column = index % HW_NR_OF_BUTTON_COLUMNS;
        uint8_t row    = (index / HW_NR_OF_BUTTON_COLUMNS) * 2;
        index          = row * HW_NR_OF_BUTTON_COLUMNS + column;

        if (component == encoderComponent_t::A)
        {
            return index;
        }

        return index + HW_NR_OF_BUTTON_COLUMNS;
    }
}    // namespace Board::IO::digitalIn

#include "Common.cpp.include"

#endif
#endif