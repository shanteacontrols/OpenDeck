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

#ifdef PROJECT_TARGET_SUPPORT_DIGITAL_INPUTS
#ifdef PROJECT_TARGET_DRIVER_DIGITAL_INPUT_MATRIX_NATIVE_ROWS

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "board/Board.h"
#include "board/src/Internal.h"
#include "core/util/Util.h"
#include "core/util/RingBuffer.h"
#include <Target.h>

using namespace board::io::digitalIn;
using namespace board::detail;
using namespace board::detail::io::digitalIn;

namespace
{
    volatile readings_t _digitalInBuffer[PROJECT_TARGET_MAX_NR_OF_DIGITAL_INPUTS];
    volatile uint8_t    _activeInColumn;

    inline void activateInputColumn()
    {
        CORE_MCU_IO_SET_STATE(PIN_PORT_DEC_BM_A0, PIN_INDEX_DEC_BM_A0, core::util::BIT_READ(_activeInColumn, 0));
        CORE_MCU_IO_SET_STATE(PIN_PORT_DEC_BM_A1, PIN_INDEX_DEC_BM_A1, core::util::BIT_READ(_activeInColumn, 1));
        CORE_MCU_IO_SET_STATE(PIN_PORT_DEC_BM_A2, PIN_INDEX_DEC_BM_A2, core::util::BIT_READ(_activeInColumn, 2));

        if (++_activeInColumn == PROJECT_TARGET_NR_OF_BUTTON_COLUMNS)
        {
            _activeInColumn = 0;
        }
    }

    inline void storeDigitalIn()
    {
        for (uint8_t column = 0; column < PROJECT_TARGET_NR_OF_BUTTON_COLUMNS; column++)
        {
            activateInputColumn();

            core::mcu::io::pin_t pin;

            for (uint8_t row = 0; row < PROJECT_TARGET_NR_OF_BUTTON_ROWS; row++)
            {
                size_t index = (row * PROJECT_TARGET_NR_OF_BUTTON_COLUMNS) + column;
                pin          = map::BUTTON_PIN(row);

                _digitalInBuffer[index].readings <<= 1;
                _digitalInBuffer[index].readings |= !CORE_MCU_IO_READ(pin.port, pin.index);

                if (++_digitalInBuffer[index].count > MAX_READING_COUNT)
                {
                    _digitalInBuffer[index].count = MAX_READING_COUNT;
                }
            }
        }
    }
}    // namespace

namespace board::detail::io::digitalIn
{
    void init()
    {
        for (uint8_t i = 0; i < PROJECT_TARGET_NR_OF_BUTTON_ROWS; i++)
        {
            auto pin = detail::map::BUTTON_PIN(i);

#ifndef PROJECT_TARGET_BUTTONS_EXT_PULLUPS
            CORE_MCU_IO_INIT(pin.port, pin.index, core::mcu::io::pinMode_t::INPUT, core::mcu::io::pullMode_t::UP);
#else
            CORE_MCU_IO_INIT(pin.port, pin.index, core::mcu::io::pinMode_t::INPUT, core::mcu::io::pullMode_t::NONE);
#endif
        }

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
}    // namespace board::detail::io::digitalIn

namespace board::io::digitalIn
{
    bool state(size_t index, readings_t& readings)
    {
        if (index >= PROJECT_TARGET_MAX_NR_OF_DIGITAL_INPUTS)
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
        uint8_t row    = index / PROJECT_TARGET_NR_OF_BUTTON_COLUMNS;
        uint8_t column = index % PROJECT_TARGET_NR_OF_BUTTON_COLUMNS;

        if (row % 2)
        {
            row -= 1;    // uneven row, get info from previous (even) row
        }

        return (row * PROJECT_TARGET_NR_OF_BUTTON_COLUMNS) / 2 + column;
    }

    size_t encoderComponentFromEncoder(size_t index, encoderComponent_t component)
    {
        uint8_t column = index % PROJECT_TARGET_NR_OF_BUTTON_COLUMNS;
        uint8_t row    = (index / PROJECT_TARGET_NR_OF_BUTTON_COLUMNS) * 2;
        index          = row * PROJECT_TARGET_NR_OF_BUTTON_COLUMNS + column;

        if (component == encoderComponent_t::A)
        {
            return index;
        }

        return index + PROJECT_TARGET_NR_OF_BUTTON_COLUMNS;
    }
}    // namespace board::io::digitalIn

#include "Common.cpp.include"

#endif
#endif