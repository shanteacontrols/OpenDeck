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
#ifdef DIGITAL_INPUT_DRIVER_MATRIX_SHIFT_REGISTER_ROWS

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "board/Board.h"
#include "board/Internal.h"
#include "board/common/constants/IO.h"
#include "core/src/util/Util.h"
#include "core/src/util/RingBuffer.h"
#include <Target.h>

using namespace Board::IO::digitalIn;
using namespace Board::detail;
using namespace Board::detail::IO::digitalIn;

namespace
{
    volatile readings_t _digitalInBuffer[NR_OF_DIGITAL_INPUTS];
    volatile uint8_t    _activeInColumn;

    inline void activateInputColumn()
    {
        core::util::BIT_READ(_activeInColumn, 0) ? CORE_MCU_IO_SET_HIGH(DEC_BM_PORT_A0, DEC_BM_PIN_A0) : CORE_MCU_IO_SET_LOW(DEC_BM_PORT_A0, DEC_BM_PIN_A0);
        core::util::BIT_READ(_activeInColumn, 1) ? CORE_MCU_IO_SET_HIGH(DEC_BM_PORT_A1, DEC_BM_PIN_A1) : CORE_MCU_IO_SET_LOW(DEC_BM_PORT_A1, DEC_BM_PIN_A1);
        core::util::BIT_READ(_activeInColumn, 2) ? CORE_MCU_IO_SET_HIGH(DEC_BM_PORT_A2, DEC_BM_PIN_A2) : CORE_MCU_IO_SET_LOW(DEC_BM_PORT_A2, DEC_BM_PIN_A2);

        if (++_activeInColumn == NUMBER_OF_BUTTON_COLUMNS)
        {
            _activeInColumn = 0;
        }
    }

    inline void storeDigitalIn()
    {
        for (uint8_t column = 0; column < NUMBER_OF_BUTTON_COLUMNS; column++)
        {
            activateInputColumn();

            IO::spiWait();
            CORE_MCU_IO_SET_LOW(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
            CORE_MCU_IO_SET_LOW(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);
            IO::spiWait();

            CORE_MCU_IO_SET_HIGH(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);

            for (uint8_t row = 0; row < NUMBER_OF_BUTTON_ROWS; row++)
            {
                // this register shifts out MSB first
                size_t index = ((((NUMBER_OF_IN_SR * 8) - 1) - row) * NUMBER_OF_BUTTON_COLUMNS) + column;
                CORE_MCU_IO_SET_LOW(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
                IO::spiWait();

                _digitalInBuffer[index].readings <<= 1;
                _digitalInBuffer[index].readings |= !CORE_MCU_IO_READ(SR_IN_DATA_PORT, SR_IN_DATA_PIN);

                if (++_digitalInBuffer[index].count > MAX_READING_COUNT)
                {
                    _digitalInBuffer[index].count = MAX_READING_COUNT;
                }

                CORE_MCU_IO_SET_HIGH(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
            }
        }
    }
}    // namespace

namespace Board::detail::IO::digitalIn
{
    void init()
    {
        CORE_MCU_IO_INIT(SR_IN_DATA_PORT, SR_IN_DATA_PIN, core::mcu::io::pinMode_t::INPUT);
        CORE_MCU_IO_INIT(SR_IN_CLK_PORT, SR_IN_CLK_PIN, core::mcu::io::pinMode_t::OUTPUT_PP);
        CORE_MCU_IO_INIT(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN, core::mcu::io::pinMode_t::OUTPUT_PP);

        CORE_MCU_IO_SET_LOW(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
        CORE_MCU_IO_SET_HIGH(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);

        CORE_MCU_IO_INIT(DEC_BM_PORT_A0, DEC_BM_PIN_A0, core::mcu::io::pinMode_t::OUTPUT_PP, core::mcu::io::pullMode_t::NONE);
        CORE_MCU_IO_INIT(DEC_BM_PORT_A1, DEC_BM_PIN_A1, core::mcu::io::pinMode_t::OUTPUT_PP, core::mcu::io::pullMode_t::NONE);
        CORE_MCU_IO_INIT(DEC_BM_PORT_A2, DEC_BM_PIN_A2, core::mcu::io::pinMode_t::OUTPUT_PP, core::mcu::io::pullMode_t::NONE);

        CORE_MCU_IO_SET_LOW(DEC_BM_PORT_A0, DEC_BM_PIN_A0);
        CORE_MCU_IO_SET_LOW(DEC_BM_PORT_A1, DEC_BM_PIN_A1);
        CORE_MCU_IO_SET_LOW(DEC_BM_PORT_A2, DEC_BM_PIN_A2);
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

        index = map::buttonIndex(index);

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
        uint8_t row    = index / NUMBER_OF_BUTTON_COLUMNS;
        uint8_t column = index % NUMBER_OF_BUTTON_COLUMNS;

        if (row % 2)
        {
            row -= 1;    // uneven row, get info from previous (even) row
        }

        return (row * NUMBER_OF_BUTTON_COLUMNS) / 2 + column;
    }

    size_t encoderComponentFromEncoder(size_t index, encoderComponent_t component)
    {
        uint8_t column = index % NUMBER_OF_BUTTON_COLUMNS;
        uint8_t row    = (index / NUMBER_OF_BUTTON_COLUMNS) * 2;
        index          = row * NUMBER_OF_BUTTON_COLUMNS + column;

        if (component == encoderComponent_t::A)
        {
            return index;
        }

        return index + NUMBER_OF_BUTTON_COLUMNS;
    }
}    // namespace Board::IO::digitalIn

#include "Common.cpp.include"

#endif
#endif