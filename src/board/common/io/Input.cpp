/*

Copyright 2015-2020 Igor Petrovic

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
    volatile uint8_t digitalInBuffer[DIGITAL_IN_BUFFER_SIZE][DIGITAL_IN_ARRAY_SIZE];
    uint8_t          digitalInBufferReadOnly[DIGITAL_IN_ARRAY_SIZE];

#ifdef NUMBER_OF_BUTTON_COLUMNS
    volatile uint8_t activeInColumn;
#endif

    volatile uint8_t dIn_head;
    volatile uint8_t dIn_tail;
    volatile uint8_t dIn_count;

#if defined(SR_IN_CLK_PORT) && defined(SR_IN_LATCH_PORT) && defined(SR_IN_DATA_PORT) && !defined(NUMBER_OF_BUTTON_COLUMNS) && !defined(NUMBER_OF_BUTTON_ROWS)
    inline void storeDigitalIn()
    {
        CORE_IO_SET_LOW(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
        CORE_IO_SET_LOW(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);
        Board::detail::io::sr165wait();

        CORE_IO_SET_HIGH(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);

        for (int j = 0; j < NUMBER_OF_IN_SR; j++)
        {
            for (int i = 0; i < 8; i++)
            {
                CORE_IO_SET_LOW(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
                Board::detail::io::sr165wait();
                BIT_WRITE(digitalInBuffer[dIn_head][j], 7 - i, !CORE_IO_READ(SR_IN_DATA_PORT, SR_IN_DATA_PIN));
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

    ///
    /// Acquires data for all buttons connected in currently active button matrix column by
    /// reading inputs from shift register.
    ///
    inline void storeDigitalIn()
    {
        for (int i = 0; i < NUMBER_OF_BUTTON_COLUMNS; i++)
        {
            activateInputColumn();

#ifdef NUMBER_OF_IN_SR
            Board::detail::io::sr165wait();
            CORE_IO_SET_LOW(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
            CORE_IO_SET_LOW(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);
            Board::detail::io::sr165wait();

            CORE_IO_SET_HIGH(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);

            for (int j = 0; j < NUMBER_OF_BUTTON_ROWS; j++)
            {
                CORE_IO_SET_LOW(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
                Board::detail::io::sr165wait();
                BIT_WRITE(digitalInBuffer[dIn_head][i], j, !CORE_IO_READ(SR_IN_DATA_PORT, SR_IN_DATA_PIN));
                CORE_IO_SET_HIGH(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
            }
#else
            core::io::mcuPin_t pin;

            for (int j = 0; j < NUMBER_OF_BUTTON_ROWS; j++)
            {
                pin = Board::detail::map::buttonPin(j);
                BIT_WRITE(digitalInBuffer[dIn_head][i], j, !CORE_IO_READ(CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin)));
            }
#endif
        }
    }
#else
    core::io::mcuPin_t pin;

    inline void storeDigitalIn()
    {
        for (int i = 0; i < DIGITAL_IN_ARRAY_SIZE; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                uint8_t buttonIndex = i * 8 + j;

                if (buttonIndex >= MAX_NUMBER_OF_BUTTONS)
                    break;    //done

                pin = Board::detail::map::buttonPin(buttonIndex);

                BIT_WRITE(digitalInBuffer[dIn_head][i], j, !CORE_IO_READ(CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin)));
            }
        }
    }
#endif
}    // namespace

namespace Board
{
    namespace io
    {
        bool getButtonState(uint8_t buttonID)
        {
            if (buttonID >= MAX_NUMBER_OF_BUTTONS)
                return false;

            buttonID = detail::map::buttonIndex(buttonID);

#ifdef NUMBER_OF_BUTTON_COLUMNS
            uint8_t row    = buttonID / NUMBER_OF_BUTTON_COLUMNS;
            uint8_t column = buttonID % NUMBER_OF_BUTTON_COLUMNS;

            return BIT_READ(digitalInBufferReadOnly[column], row);
#else
            uint8_t arrayIndex  = buttonID / 8;
            uint8_t buttonIndex = buttonID - 8 * arrayIndex;

            return BIT_READ(digitalInBufferReadOnly[arrayIndex], buttonIndex);
#endif
        }

        uint8_t getEncoderPair(uint8_t buttonID)
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

        uint8_t getEncoderPairState(uint8_t encoderID)
        {
#ifdef NUMBER_OF_BUTTON_COLUMNS
            uint8_t column = encoderID % NUMBER_OF_BUTTON_COLUMNS;
            uint8_t row    = (encoderID / NUMBER_OF_BUTTON_COLUMNS) * 2;

            uint8_t buttonID;

            buttonID = row * NUMBER_OF_BUTTON_COLUMNS + column;

            uint8_t pairState = getButtonState(buttonID);
            pairState <<= 1;
            pairState |= getButtonState(buttonID + NUMBER_OF_BUTTON_COLUMNS);
#else
            uint8_t buttonID = encoderID * 2;

            uint8_t pairState = getButtonState(buttonID);
            pairState <<= 1;
            pairState |= getButtonState(buttonID + 1);
#endif

            return pairState;
        }

        bool isInputDataAvailable()
        {
            if (dIn_count)
            {
                ATOMIC_SECTION
                {
                    if (++dIn_tail == DIGITAL_IN_BUFFER_SIZE)
                        dIn_tail = 0;

                    for (int i = 0; i < DIGITAL_IN_ARRAY_SIZE; i++)
                        digitalInBufferReadOnly[i] = digitalInBuffer[dIn_tail][i];

                    dIn_count--;
                }

                return true;
            }

            return false;
        }
    }    // namespace io

    namespace detail
    {
        namespace io
        {
            void checkDigitalInputs()
            {
                if (dIn_count < DIGITAL_IN_BUFFER_SIZE)
                {
                    if (++dIn_head == DIGITAL_IN_BUFFER_SIZE)
                        dIn_head = 0;

                    storeDigitalIn();

                    dIn_count++;
                }
            }
        }    // namespace io
    }        // namespace detail
}    // namespace Board