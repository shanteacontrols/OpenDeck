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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "board/Board.h"
#include "board/Internal.h"
#include "board/common/constants/IO.h"
#include "core/src/general/Helpers.h"
#include "core/src/general/Atomic.h"
#include "core/src/general/RingBuffer.h"
#include <Target.h>

#define MAX_READING_COUNT (8 * sizeof(((Board::IO::dInReadings_t*)0)->readings))

namespace
{
    volatile Board::IO::dInReadings_t _digitalInBuffer[NR_OF_DIGITAL_INPUTS];
#ifdef NATIVE_BUTTON_INPUTS
    core::RingBuffer<core::io::portWidth_t, MAX_READING_COUNT> _portBuffer[NR_OF_DIGITAL_INPUT_PORTS];
#endif

#ifdef NUMBER_OF_BUTTON_COLUMNS
    volatile uint8_t _activeInColumn;
#endif

#if defined(SR_IN_CLK_PORT) && defined(SR_IN_LATCH_PORT) && defined(SR_IN_DATA_PORT) && !defined(NUMBER_OF_BUTTON_COLUMNS) && !defined(NUMBER_OF_BUTTON_ROWS)
    inline void storeDigitalIn()
    {
        CORE_IO_SET_LOW(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
        CORE_IO_SET_LOW(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);
        Board::detail::IO::sr165wait();

        CORE_IO_SET_HIGH(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);

        for (int shiftRegister = 0; shiftRegister < NUMBER_OF_IN_SR; shiftRegister++)
        {
            for (int input = 0; input < 8; input++)
            {
                // this register shifts out MSB first
                size_t buttonIndex = (shiftRegister * 8) + (7 - input);
                CORE_IO_SET_LOW(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
                Board::detail::IO::sr165wait();

                _digitalInBuffer[buttonIndex].readings <<= 1;
                _digitalInBuffer[buttonIndex].readings |= !CORE_IO_READ(SR_IN_DATA_PORT, SR_IN_DATA_PIN);

                if (++_digitalInBuffer[buttonIndex].count > MAX_READING_COUNT)
                {
                    _digitalInBuffer[buttonIndex].count = MAX_READING_COUNT;
                }

                CORE_IO_SET_HIGH(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
            }
        }
    }
#elif defined(NUMBER_OF_BUTTON_COLUMNS) && defined(NUMBER_OF_BUTTON_ROWS)
    inline void activateInputColumn()
    {
        BIT_READ(_activeInColumn, 0) ? CORE_IO_SET_HIGH(DEC_BM_PORT_A0, DEC_BM_PIN_A0) : CORE_IO_SET_LOW(DEC_BM_PORT_A0, DEC_BM_PIN_A0);
        BIT_READ(_activeInColumn, 1) ? CORE_IO_SET_HIGH(DEC_BM_PORT_A1, DEC_BM_PIN_A1) : CORE_IO_SET_LOW(DEC_BM_PORT_A1, DEC_BM_PIN_A1);
        BIT_READ(_activeInColumn, 2) ? CORE_IO_SET_HIGH(DEC_BM_PORT_A2, DEC_BM_PIN_A2) : CORE_IO_SET_LOW(DEC_BM_PORT_A2, DEC_BM_PIN_A2);

        if (++_activeInColumn == NUMBER_OF_BUTTON_COLUMNS)
        {
            _activeInColumn = 0;
        }
    }

    /// Acquires data for all buttons connected in currently active button matrix column by
    /// reading inputs from shift register.
    inline void storeDigitalIn()
    {
        for (int column = 0; column < NUMBER_OF_BUTTON_COLUMNS; column++)
        {
            activateInputColumn();

#ifdef NUMBER_OF_IN_SR
            Board::detail::IO::sr165wait();
            CORE_IO_SET_LOW(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
            CORE_IO_SET_LOW(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);
            Board::detail::IO::sr165wait();

            CORE_IO_SET_HIGH(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);

            for (int row = 0; row < NUMBER_OF_BUTTON_ROWS; row++)
            {
                // this register shifts out MSB first
                size_t buttonIndex = ((7 - row) * 8) + column;
                CORE_IO_SET_LOW(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
                Board::detail::IO::sr165wait();

                _digitalInBuffer[buttonIndex].readings <<= 1;
                _digitalInBuffer[buttonIndex].readings |= !CORE_IO_READ(SR_IN_DATA_PORT, SR_IN_DATA_PIN);

                if (++_digitalInBuffer[buttonIndex].count > MAX_READING_COUNT)
                    _digitalInBuffer[buttonIndex].count = MAX_READING_COUNT;

                CORE_IO_SET_HIGH(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
            }
#else
            core::io::mcuPin_t pin;

            for (int row = 0; row < NUMBER_OF_BUTTON_ROWS; row++)
            {
                size_t buttonIndex = (row * 8) + column;
                pin                = Board::detail::map::buttonPin(row);

                _digitalInBuffer[buttonIndex].readings <<= 1;
                _digitalInBuffer[buttonIndex].readings |= !CORE_IO_READ(CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin));

                if (++_digitalInBuffer[buttonIndex].count > MAX_READING_COUNT)
                {
                    _digitalInBuffer[buttonIndex].count = MAX_READING_COUNT;
                }
            }
#endif
        }
    }
#else
    inline void storeDigitalIn()
    {
        // read all input ports instead of reading pin by pin to reduce the time spent in ISR
        for (int portIndex = 0; portIndex < NR_OF_DIGITAL_INPUT_PORTS; portIndex++)
        {
            _portBuffer[portIndex].insert(CORE_IO_READ_IN_PORT(Board::detail::map::digitalInPort(portIndex)));
        }
    }

    inline void fillBuffer(size_t digitalInIndex)
    {
        // for provided button index, retrieve its port index
        // upon reading update all buttons located on that port

        auto                  portIndex = Board::detail::map::buttonPortIndex(digitalInIndex);
        core::io::portWidth_t portValue = 0;

        while (_portBuffer[portIndex].remove(portValue))
        {
            for (int i = 0; i < NR_OF_DIGITAL_INPUTS; i++)
            {
                if (Board::detail::map::buttonPortIndex(i) == portIndex)
                {
                    _digitalInBuffer[i].readings <<= 1;
                    _digitalInBuffer[i].readings |= !BIT_READ(portValue, Board::detail::map::buttonPinIndex(i));

                    if (++_digitalInBuffer[i].count > MAX_READING_COUNT)
                    {
                        _digitalInBuffer[i].count = MAX_READING_COUNT;
                    }
                }
            }
        }
    }
#endif
}    // namespace

namespace Board
{
    namespace IO
    {
        bool digitalInState(size_t digitalInIndex, dInReadings_t& dInReadings)
        {
            if (digitalInIndex >= NR_OF_DIGITAL_INPUTS)
            {
                return false;
            }

#ifdef NATIVE_BUTTON_INPUTS
            fillBuffer(digitalInIndex);
#endif

            digitalInIndex = detail::map::buttonIndex(digitalInIndex);

#ifndef NATIVE_BUTTON_INPUTS
            ATOMIC_SECTION
#endif
            {
                dInReadings.count                      = _digitalInBuffer[digitalInIndex].count;
                dInReadings.readings                   = _digitalInBuffer[digitalInIndex].readings;
                _digitalInBuffer[digitalInIndex].count = 0;
            }

            return dInReadings.count > 0;
        }

        size_t encoderIndex(size_t buttonID)
        {
#ifdef NUMBER_OF_BUTTON_COLUMNS
            uint8_t row    = buttonID / NUMBER_OF_BUTTON_COLUMNS;
            uint8_t column = buttonID % NUMBER_OF_BUTTON_COLUMNS;

            if (row % 2)
            {
                row -= 1;    // uneven row, get info from previous (even) row
            }

            return (row * NUMBER_OF_BUTTON_COLUMNS) / 2 + column;
#else
            return buttonID / 2;
#endif
        }

        size_t encoderSignalIndex(size_t encoderID, encoderIndex_t index)
        {
#ifdef NUMBER_OF_BUTTON_COLUMNS
            uint8_t column = encoderID % NUMBER_OF_BUTTON_COLUMNS;
            uint8_t row    = (encoderID / NUMBER_OF_BUTTON_COLUMNS) * 2;

            uint8_t buttonID = row * NUMBER_OF_BUTTON_COLUMNS + column;

            if (index == encoderIndex_t::A)
            {
                return buttonID;
            }

            return buttonID + NUMBER_OF_BUTTON_COLUMNS;
#else
            uint8_t buttonID = encoderID * 2;

            if (index == encoderIndex_t::A)
            {
                return buttonID;
            }

            return buttonID + 1;

#endif
        }
    }    // namespace IO

    namespace detail::IO
    {
        void checkDigitalInputs()
        {
            storeDigitalIn();
        }

        void flushInputReadings()
        {
            ATOMIC_SECTION
            {
                for (size_t i = 0; i < NR_OF_DIGITAL_INPUTS; i++)
                {
                    _digitalInBuffer[i].count = 0;
                }
            }
        }
    }    // namespace detail::IO
}    // namespace Board