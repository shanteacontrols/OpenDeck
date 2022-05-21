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

#ifdef DIGITAL_OUTPUTS_SUPPORTED
#ifdef DIGITAL_OUTPUT_DRIVER_MATRIX_NATIVE_ROWS

#include "board/Board.h"
#include "board/common/io/Helpers.h"
#include "board/common/constants/IO.h"
#include "board/Internal.h"
#include "core/src/util/Util.h"
#include <Target.h>

namespace
{
    uint8_t          _pwmCounter;
    volatile uint8_t _ledState[(NR_OF_DIGITAL_OUTPUTS / 8) + 1][static_cast<uint8_t>(Board::IO::ledBrightness_t::B100)];

    enum class switchState_t : uint8_t
    {
        NONE,
        ROWS_OFF,
        COLUMNS
    };

    switchState_t switchState;

    /// Holds value of currently active output matrix column.
    volatile uint8_t activeOutColumn;

    /// Used to turn the given LED row off.
    inline void ledRowOff(uint8_t row)
    {
        core::mcu::io::pin_t pin = Board::detail::map::ledPin(row);
        EXT_LED_OFF(CORE_MCU_IO_PIN_PORT(pin), CORE_MCU_IO_PIN_INDEX(pin));
    }

    inline void ledRowOn(uint8_t row)
    {
        core::mcu::io::pin_t pin = Board::detail::map::ledPin(row);
        EXT_LED_ON(CORE_MCU_IO_PIN_PORT(pin), CORE_MCU_IO_PIN_INDEX(pin));
    }
}    // namespace

namespace Board::detail::IO
{
    void checkDigitalOutputs()
    {
        switch (switchState)
        {
        case switchState_t::NONE:
        {
            if (++_pwmCounter >= (static_cast<uint8_t>(Board::IO::ledBrightness_t::B100) - 1))
            {
                switchState = switchState_t::ROWS_OFF;
            }
        }
        break;

        case switchState_t::ROWS_OFF:
        {
            _pwmCounter = 0;

            for (int i = 0; i < NUMBER_OF_LED_ROWS; i++)
            {
                ledRowOff(i);
            }

            switchState = switchState_t::COLUMNS;

            // allow some settle time to avoid near LEDs being slighty lit
            return;
        }
        break;

        case switchState_t::COLUMNS:
        {
            if (++activeOutColumn >= NUMBER_OF_LED_COLUMNS)
            {
                activeOutColumn = 0;
            }

            core::util::BIT_READ(activeOutColumn, 0) ? CORE_MCU_IO_SET_HIGH(DEC_LM_PORT_A0, DEC_LM_PIN_A0) : CORE_MCU_IO_SET_LOW(DEC_LM_PORT_A0, DEC_LM_PIN_A0);
            core::util::BIT_READ(activeOutColumn, 1) ? CORE_MCU_IO_SET_HIGH(DEC_LM_PORT_A1, DEC_LM_PIN_A1) : CORE_MCU_IO_SET_LOW(DEC_LM_PORT_A1, DEC_LM_PIN_A1);
            core::util::BIT_READ(activeOutColumn, 2) ? CORE_MCU_IO_SET_HIGH(DEC_LM_PORT_A2, DEC_LM_PIN_A2) : CORE_MCU_IO_SET_LOW(DEC_LM_PORT_A2, DEC_LM_PIN_A2);

            switchState = switchState_t::NONE;
        }
        break;
        }

        for (int i = 0; i < NUMBER_OF_LED_ROWS; i++)
        {
            size_t  ledID      = activeOutColumn + i * NUMBER_OF_LED_COLUMNS;
            uint8_t arrayIndex = ledID / 8;
            uint8_t ledBit     = ledID - 8 * arrayIndex;

            core::util::BIT_READ(_ledState[arrayIndex][_pwmCounter], ledBit) ? ledRowOn(i) : ledRowOff(i);
        }
    }
}    // namespace Board::detail::IO

namespace Board::IO
{
    void writeLEDstate(size_t ledID, IO::ledBrightness_t ledBrightness)
    {
        if (ledID >= NR_OF_DIGITAL_OUTPUTS)
        {
            return;
        }

        ledID = detail::map::ledIndex(ledID);

        CORE_MCU_ATOMIC_SECTION
        {
            for (int i = 0; i < static_cast<int>(ledBrightness_t::B100); i++)
            {
                uint8_t arrayIndex = ledID / 8;
                uint8_t ledBit     = ledID - 8 * arrayIndex;

                core::util::BIT_WRITE(_ledState[arrayIndex][i], ledBit, i < static_cast<int>(ledBrightness) ? 1 : 0);
            }
        }
    }

    size_t rgbSignalIndex(size_t rgbID, Board::IO::rgbIndex_t index)
    {
        uint8_t column  = rgbID % NUMBER_OF_LED_COLUMNS;
        uint8_t row     = (rgbID / NUMBER_OF_LED_COLUMNS) * 3;
        uint8_t address = column + NUMBER_OF_LED_COLUMNS * row;

        switch (index)
        {
        case rgbIndex_t::R:
            return address;

        case rgbIndex_t::G:
            return address + NUMBER_OF_LED_COLUMNS * 1;

        case rgbIndex_t::B:
            return address + NUMBER_OF_LED_COLUMNS * 2;
        }

        return 0;
    }

    size_t rgbIndex(size_t ledID)
    {
        uint8_t row = ledID / NUMBER_OF_LED_COLUMNS;

        uint8_t mod = row % 3;
        row -= mod;

        uint8_t column = ledID % NUMBER_OF_LED_COLUMNS;

        uint8_t result = (row * NUMBER_OF_LED_COLUMNS) / 3 + column;

        if (result >= Board::detail::IO::NR_OF_RGB_LEDS)
        {
            return Board::detail::IO::NR_OF_RGB_LEDS - 1;
        }

        return result;
    }
}    // namespace Board::IO

#endif
#endif