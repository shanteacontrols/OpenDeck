/*

Copyright 2015-2021 Igor Petrovic

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
#include "Pins.h"

namespace
{
    volatile Board::io::dInReadings_t digitalInBuffer[MAX_NUMBER_OF_BUTTONS];

#ifdef NUMBER_OF_BUTTON_COLUMNS
    volatile uint8_t activeInColumn;
#endif

#if defined(SR_IN_CLK_PORT) && defined(SR_IN_LATCH_PORT) && defined(SR_IN_DATA_PORT) && !defined(NUMBER_OF_BUTTON_COLUMNS) && !defined(NUMBER_OF_BUTTON_ROWS)
    inline void storeDigitalIn()
    {
        CORE_IO_SET_LOW(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
        CORE_IO_SET_LOW(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);
        Board::detail::io::sr165wait();

        CORE_IO_SET_HIGH(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);

        for (int shiftRegister = 0; shiftRegister < NUMBER_OF_IN_SR; shiftRegister++)
        {
            for (int input = 0; input < 8; input++)
            {
                //this register shifts out MSB first
                size_t buttonIndex = (shiftRegister * 8) + (7 - input);
                CORE_IO_SET_LOW(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
                Board::detail::io::sr165wait();

                digitalInBuffer[buttonIndex].readings <<= 1;
                digitalInBuffer[buttonIndex].readings |= !CORE_IO_READ(SR_IN_DATA_PORT, SR_IN_DATA_PIN);

                if (++digitalInBuffer[buttonIndex].count > 32)
                    digitalInBuffer[buttonIndex].count = 32;

                CORE_IO_SET_HIGH(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
            }
        }
    }
#elif defined(NUMBER_OF_BUTTON_COLUMNS) && defined(NUMBER_OF_BUTTON_ROWS)
    inline void activateInputColumn()
    {
        BIT_READ(activeInColumn, 0) ? CORE_IO_SET_HIGH(DEC_BM_PORT_A0, DEC_BM_PIN_A0) : CORE_IO_SET_LOW(DEC_BM_PORT_A0, DEC_BM_PIN_A0);
        BIT_READ(activeInColumn, 1) ? CORE_IO_SET_HIGH(DEC_BM_PORT_A1, DEC_BM_PIN_A1) : CORE_IO_SET_LOW(DEC_BM_PORT_A1, DEC_BM_PIN_A1);
        BIT_READ(activeInColumn, 2) ? CORE_IO_SET_HIGH(DEC_BM_PORT_A2, DEC_BM_PIN_A2) : CORE_IO_SET_LOW(DEC_BM_PORT_A2, DEC_BM_PIN_A2);

        if (++activeInColumn == NUMBER_OF_BUTTON_COLUMNS)
            activeInColumn = 0;
    }

    /// Acquires data for all buttons connected in currently active button matrix column by
    /// reading inputs from shift register.
    inline void storeDigitalIn()
    {
        for (int column = 0; column < NUMBER_OF_BUTTON_COLUMNS; column++)
        {
            activateInputColumn();

#ifdef NUMBER_OF_IN_SR
            Board::detail::io::sr165wait();
            CORE_IO_SET_LOW(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
            CORE_IO_SET_LOW(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);
            Board::detail::io::sr165wait();

            CORE_IO_SET_HIGH(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);

            for (int row = 0; row < NUMBER_OF_BUTTON_ROWS; row++)
            {
                //this register shifts out MSB first
                size_t buttonIndex = ((7 - row) * 8) + column;
                CORE_IO_SET_LOW(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
                Board::detail::io::sr165wait();

                digitalInBuffer[buttonIndex].readings <<= 1;
                digitalInBuffer[buttonIndex].readings |= !CORE_IO_READ(SR_IN_DATA_PORT, SR_IN_DATA_PIN);

                if (++digitalInBuffer[buttonIndex].count > 32)
                    digitalInBuffer[buttonIndex].count = 32;

                CORE_IO_SET_HIGH(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
            }
#else
            core::io::mcuPin_t pin;

            for (int row = 0; row < NUMBER_OF_BUTTON_ROWS; row++)
            {
                size_t buttonIndex = (row * 8) + column;
                pin                = Board::detail::map::buttonPin(row);

                digitalInBuffer[buttonIndex].readings <<= 1;
                digitalInBuffer[buttonIndex].readings |= !CORE_IO_READ(CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin));

                if (++digitalInBuffer[buttonIndex].count > 32)
                    digitalInBuffer[buttonIndex].count = 32;
            }
#endif
        }
    }
#else
    core::io::mcuPin_t pin;

    inline void storeDigitalIn()
    {
        for (int buttonIndex = 0; buttonIndex < MAX_NUMBER_OF_BUTTONS; buttonIndex++)
        {
            pin = Board::detail::map::buttonPin(buttonIndex);

            digitalInBuffer[buttonIndex].readings <<= 1;
            digitalInBuffer[buttonIndex].readings |= !CORE_IO_READ(CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin));

            if (++digitalInBuffer[buttonIndex].count > 32)
                digitalInBuffer[buttonIndex].count = 32;
        }
    }
#endif
}    // namespace

namespace Board
{
    namespace io
    {
        bool digitalInState(size_t digitalInIndex, dInReadings_t& dInReadings)
        {
            if (digitalInIndex >= MAX_NUMBER_OF_BUTTONS)
                return false;

            digitalInIndex = detail::map::buttonIndex(digitalInIndex);

            ATOMIC_SECTION
            {
                dInReadings.count                     = digitalInBuffer[digitalInIndex].count;
                dInReadings.readings                  = digitalInBuffer[digitalInIndex].readings;
                digitalInBuffer[digitalInIndex].count = 0;
            }

            return dInReadings.count > 0;
        }

        size_t encoderIndex(size_t buttonID)
        {
#ifdef NUMBER_OF_BUTTON_COLUMNS
            uint8_t row    = buttonID / NUMBER_OF_BUTTON_COLUMNS;
            uint8_t column = buttonID % NUMBER_OF_BUTTON_COLUMNS;

            if (row % 2)
                row -= 1;    //uneven row, get info from previous (even) row

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

            if (index == encoderIndex_t::a)
                return buttonID;
            else
                return buttonID + NUMBER_OF_BUTTON_COLUMNS;
#else
            uint8_t buttonID = encoderID * 2;

            if (index == encoderIndex_t::a)
                return buttonID;
            else
                return buttonID + 1;
#endif
        }
    }    // namespace io

    namespace detail
    {
        namespace io
        {
            void checkDigitalInputs()
            {
                storeDigitalIn();
            }

            void flushInputReadings()
            {
                ATOMIC_SECTION
                {
                    for (size_t i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
                        digitalInBuffer[i].count = 0;
                }
            }
        }    // namespace io
    }        // namespace detail
}    // namespace Board