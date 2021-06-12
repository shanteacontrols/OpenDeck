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

#include "board/Board.h"
#include "board/common/io/Helpers.h"
#include "board/common/constants/IO.h"
#include "board/Internal.h"
#include "core/src/general/Helpers.h"
#include "core/src/general/Atomic.h"
#include "Pins.h"

namespace
{
#if !defined(NUMBER_OF_OUT_SR) || defined(NUMBER_OF_LED_COLUMNS)
    core::io::mcuPin_t pin;
#endif
    uint8_t          ledID;
    uint8_t          arrayIndex;
    uint8_t          ledBit;
    uint8_t          pwmCounter;
    volatile uint8_t ledState[(MAX_NUMBER_OF_LEDS / 8) + 1][static_cast<uint8_t>(Board::io::ledBrightness_t::b100)];

#ifdef NUMBER_OF_LED_COLUMNS
    /// Holds value of currently active output matrix column.
    volatile uint8_t activeOutColumn = NUMBER_OF_LED_COLUMNS;

    /// Used to turn the given LED row off.
    inline void ledRowOff(uint8_t row)
    {
        pin = Board::detail::map::ledPin(row);
        EXT_LED_OFF(CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin));
    }

    inline void ledRowOn(uint8_t row)
    {
        pin = Board::detail::map::ledPin(row);
        EXT_LED_ON(CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin));
    }
#endif
}    // namespace

namespace Board
{
    namespace io
    {
        void writeLEDstate(size_t ledID, io::ledBrightness_t ledBrightness)
        {
            if (ledID >= MAX_NUMBER_OF_LEDS)
                return;

            ledID = detail::map::ledIndex(ledID);

            ATOMIC_SECTION
            {
                for (int i = 0; i < static_cast<int>(ledBrightness_t::b100); i++)
                {
                    uint8_t arrayIndex = ledID / 8;
                    uint8_t bit        = ledID - 8 * arrayIndex;

                    BIT_WRITE(ledState[arrayIndex][i], bit, i < static_cast<int>(ledBrightness) ? 1 : 0);
                }
            }
        }

        size_t rgbSignalIndex(size_t rgbID, Board::io::rgbIndex_t index)
        {
#ifdef NUMBER_OF_LED_COLUMNS
            uint8_t column  = rgbID % NUMBER_OF_LED_COLUMNS;
            uint8_t row     = (rgbID / NUMBER_OF_LED_COLUMNS) * 3;
            uint8_t address = column + NUMBER_OF_LED_COLUMNS * row;

            switch (index)
            {
            case rgbIndex_t::r:
                return address;

            case rgbIndex_t::g:
                return address + NUMBER_OF_LED_COLUMNS * 1;

            case rgbIndex_t::b:
                return address + NUMBER_OF_LED_COLUMNS * 2;
            }

            return 0;
#else
            return rgbID * 3 + static_cast<uint8_t>(index);
#endif
        }

        size_t rgbIndex(size_t ledID)
        {
#ifdef NUMBER_OF_LED_COLUMNS
            uint8_t row = ledID / NUMBER_OF_LED_COLUMNS;

            uint8_t mod = row % 3;
            row -= mod;

            uint8_t column = ledID % NUMBER_OF_LED_COLUMNS;

            uint8_t result = (row * NUMBER_OF_LED_COLUMNS) / 3 + column;

            if (result >= MAX_NUMBER_OF_RGB_LEDS)
                return MAX_NUMBER_OF_RGB_LEDS - 1;
            else
                return result;
#else
            uint8_t result = ledID / 3;

            if (result >= MAX_NUMBER_OF_RGB_LEDS)
                return MAX_NUMBER_OF_RGB_LEDS - 1;
            else
                return result;
#endif
        }
    }    // namespace io

    namespace detail
    {
        namespace io
        {
#ifdef NUMBER_OF_LED_COLUMNS
            void checkDigitalOutputs()
            {
                if (!pwmCounter)
                {
                    for (int i = 0; i < NUMBER_OF_LED_ROWS; i++)
                        ledRowOff(i);

                    if (++activeOutColumn == NUMBER_OF_LED_COLUMNS)
                        activeOutColumn = 0;

                    BIT_READ(activeOutColumn, 0) ? CORE_IO_SET_HIGH(DEC_LM_PORT_A0, DEC_LM_PIN_A0) : CORE_IO_SET_LOW(DEC_LM_PORT_A0, DEC_LM_PIN_A0);
                    BIT_READ(activeOutColumn, 1) ? CORE_IO_SET_HIGH(DEC_LM_PORT_A1, DEC_LM_PIN_A1) : CORE_IO_SET_LOW(DEC_LM_PORT_A1, DEC_LM_PIN_A1);
                    BIT_READ(activeOutColumn, 2) ? CORE_IO_SET_HIGH(DEC_LM_PORT_A2, DEC_LM_PIN_A2) : CORE_IO_SET_LOW(DEC_LM_PORT_A2, DEC_LM_PIN_A2);
                }

                for (int i = 0; i < NUMBER_OF_LED_ROWS; i++)
                {
                    ledID      = activeOutColumn + i * NUMBER_OF_LED_COLUMNS;
                    arrayIndex = ledID / 8;
                    ledBit     = ledID - 8 * arrayIndex;

                    BIT_READ(ledState[arrayIndex][pwmCounter], ledBit) ? ledRowOn(i) : ledRowOff(i);
                }

                if (++pwmCounter >= static_cast<uint8_t>(Board::io::ledBrightness_t::b100))
                    pwmCounter = 0;
            }
#elif defined(NUMBER_OF_OUT_SR)
            void checkDigitalOutputs()
            {
                CORE_IO_SET_LOW(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);

                for (int j = 0; j < NUMBER_OF_OUT_SR; j++)
                {
                    for (int i = 0; i < 8; i++)
                    {
                        ledID      = i + j * 8;
                        arrayIndex = ledID / 8;
                        ledBit     = ledID - 8 * arrayIndex;

                        BIT_READ(ledState[arrayIndex][pwmCounter], ledBit) ? EXT_LED_ON(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN) : EXT_LED_OFF(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN);
                        CORE_IO_SET_LOW(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
                        detail::io::sr595wait();
                        CORE_IO_SET_HIGH(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
                    }
                }

                CORE_IO_SET_HIGH(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);

                if (++pwmCounter >= static_cast<uint8_t>(Board::io::ledBrightness_t::b100))
                    pwmCounter = 0;
            }
#else
            void checkDigitalOutputs()
            {
                for (ledID = 0; ledID < MAX_NUMBER_OF_LEDS; ledID++)
                {
                    arrayIndex = ledID / 8;
                    ledBit     = ledID - 8 * arrayIndex;

                    pin = Board::detail::map::ledPin(ledID);
                    BIT_READ(ledState[arrayIndex][pwmCounter], ledBit) ? EXT_LED_ON(CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin)) : EXT_LED_OFF(CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin));
                }

                if (++pwmCounter >= static_cast<uint8_t>(Board::io::ledBrightness_t::b100))
                    pwmCounter = 0;
            }
#endif
        }    // namespace io
    }        // namespace detail
}    // namespace Board