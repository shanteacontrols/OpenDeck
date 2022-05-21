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

using namespace Board::detail;

namespace
{
    volatile Board::IO::dInReadings_t _digitalInBuffer[NR_OF_DIGITAL_INPUTS];
    volatile uint8_t                  _activeInColumn;

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
        for (int column = 0; column < NUMBER_OF_BUTTON_COLUMNS; column++)
        {
            activateInputColumn();

            IO::sr165wait();
            CORE_MCU_IO_SET_LOW(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
            CORE_MCU_IO_SET_LOW(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);
            IO::sr165wait();

            CORE_MCU_IO_SET_HIGH(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);

            for (int row = 0; row < NUMBER_OF_BUTTON_ROWS; row++)
            {
                // this register shifts out MSB first
                size_t buttonIndex = ((((NUMBER_OF_IN_SR * 8) - 1) - row) * NUMBER_OF_BUTTON_COLUMNS) + column;
                CORE_MCU_IO_SET_LOW(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
                IO::sr165wait();

                _digitalInBuffer[buttonIndex].readings <<= 1;
                _digitalInBuffer[buttonIndex].readings |= !CORE_MCU_IO_READ(SR_IN_DATA_PORT, SR_IN_DATA_PIN);

                if (++_digitalInBuffer[buttonIndex].count > IO::MAX_READING_COUNT)
                {
                    _digitalInBuffer[buttonIndex].count = IO::MAX_READING_COUNT;
                }

                CORE_MCU_IO_SET_HIGH(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
            }
        }
    }
}    // namespace

namespace Board::IO
{
    bool digitalInState(size_t digitalInIndex, dInReadings_t& dInReadings)
    {
        if (digitalInIndex >= NR_OF_DIGITAL_INPUTS)
        {
            return false;
        }

        digitalInIndex = detail::map::buttonIndex(digitalInIndex);

        CORE_MCU_ATOMIC_SECTION
        {
            dInReadings.count                      = _digitalInBuffer[digitalInIndex].count;
            dInReadings.readings                   = _digitalInBuffer[digitalInIndex].readings;
            _digitalInBuffer[digitalInIndex].count = 0;
        }

        return dInReadings.count > 0;
    }

    size_t encoderIndex(size_t buttonID)
    {
        uint8_t row    = buttonID / NUMBER_OF_BUTTON_COLUMNS;
        uint8_t column = buttonID % NUMBER_OF_BUTTON_COLUMNS;

        if (row % 2)
        {
            row -= 1;    // uneven row, get info from previous (even) row
        }

        return (row * NUMBER_OF_BUTTON_COLUMNS) / 2 + column;
    }

    size_t encoderSignalIndex(size_t encoderID, encoderIndex_t index)
    {
        uint8_t column = encoderID % NUMBER_OF_BUTTON_COLUMNS;
        uint8_t row    = (encoderID / NUMBER_OF_BUTTON_COLUMNS) * 2;

        uint8_t buttonID = row * NUMBER_OF_BUTTON_COLUMNS + column;

        if (index == encoderIndex_t::A)
        {
            return buttonID;
        }

        return buttonID + NUMBER_OF_BUTTON_COLUMNS;
    }
}    // namespace Board::IO

#include "Common.cpp.include"

#endif
#endif