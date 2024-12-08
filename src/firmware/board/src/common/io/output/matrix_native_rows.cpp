/*

Copyright Igor Petrovic

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

#ifdef PROJECT_TARGET_SUPPORT_DIGITAL_OUTPUTS
#ifdef PROJECT_TARGET_DRIVER_DIGITAL_OUTPUT_MATRIX_NATIVE_ROWS

#include "board/board.h"
#include "helpers.h"
#include "internal.h"
#include <target.h>

#include "core/util/util.h"

using namespace board::io::digital_out;
using namespace board::detail;
using namespace board::detail::io::digital_out;

namespace
{
    enum class switchState_t : uint8_t
    {
        NONE,
        ROWS_OFF,
        COLUMNS
    };

    uint8_t          pwmCounter;
    volatile uint8_t ledState[(PROJECT_TARGET_MAX_NR_OF_DIGITAL_OUTPUTS / 8) + 1][static_cast<uint8_t>(ledBrightness_t::B100)];

    switchState_t switchState;

    /// Holds value of currently active output matrix column.
    volatile uint8_t activeOutColumn;

    /// Used to turn the given LED row off.
    inline void ledRowOff(uint8_t row)
    {
        auto pin = map::LED_PIN(row);
        EXT_LED_OFF(pin.port, pin.index);
    }

    inline void ledRowOn(uint8_t row)
    {
        auto pin = map::LED_PIN(row);
        EXT_LED_ON(pin.port, pin.index);
    }
}    // namespace

namespace board::detail::io::digital_out
{
    void init()
    {
        CORE_MCU_IO_INIT(PIN_PORT_DEC_LM_A0,
                         PIN_INDEX_DEC_LM_A0,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);

        CORE_MCU_IO_INIT(PIN_PORT_DEC_LM_A1,
                         PIN_INDEX_DEC_LM_A1,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);

        CORE_MCU_IO_INIT(PIN_PORT_DEC_LM_A2,
                         PIN_INDEX_DEC_LM_A2,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);

        CORE_MCU_IO_SET_LOW(PIN_PORT_DEC_LM_A0, PIN_INDEX_DEC_LM_A0);
        CORE_MCU_IO_SET_LOW(PIN_PORT_DEC_LM_A1, PIN_INDEX_DEC_LM_A1);
        CORE_MCU_IO_SET_LOW(PIN_PORT_DEC_LM_A2, PIN_INDEX_DEC_LM_A2);

        for (uint8_t i = 0; i < PROJECT_TARGET_NR_OF_LED_ROWS; i++)
        {
            auto pin = detail::map::LED_PIN(i);

            // when rows are used from native outputs, use open-drain configuration
            CORE_MCU_IO_INIT(pin.port,
                             pin.index,
                             core::mcu::io::pinMode_t::OUTPUT_OD,
                             core::mcu::io::pullMode_t::NONE);

            EXT_LED_OFF(pin.port, pin.index);
        }
    }

    void update()
    {
        switch (switchState)
        {
        case switchState_t::NONE:
        {
            if (++pwmCounter >= (static_cast<uint8_t>(ledBrightness_t::B100) - 1))
            {
                switchState = switchState_t::ROWS_OFF;
            }
        }
        break;

        case switchState_t::ROWS_OFF:
        {
            pwmCounter = 0;

            for (uint8_t i = 0; i < PROJECT_TARGET_NR_OF_LED_ROWS; i++)
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
            if (++activeOutColumn >= PROJECT_TARGET_NR_OF_LED_COLUMNS)
            {
                activeOutColumn = 0;
            }

            CORE_MCU_IO_SET_STATE(PIN_PORT_DEC_LM_A0, PIN_INDEX_DEC_LM_A0, core::util::BIT_READ(activeOutColumn, 0));
            CORE_MCU_IO_SET_STATE(PIN_PORT_DEC_LM_A1, PIN_INDEX_DEC_LM_A1, core::util::BIT_READ(activeOutColumn, 1));
            CORE_MCU_IO_SET_STATE(PIN_PORT_DEC_LM_A2, PIN_INDEX_DEC_LM_A2, core::util::BIT_READ(activeOutColumn, 2));

            switchState = switchState_t::NONE;
        }
        break;
        }

        for (uint8_t i = 0; i < PROJECT_TARGET_NR_OF_LED_ROWS; i++)
        {
            size_t  index      = activeOutColumn + i * PROJECT_TARGET_NR_OF_LED_COLUMNS;
            uint8_t arrayIndex = index / 8;
            uint8_t bit        = index - 8 * arrayIndex;

            core::util::BIT_READ(ledState[arrayIndex][pwmCounter], bit) ? ledRowOn(i) : ledRowOff(i);
        }
    }
}    // namespace board::detail::io::digital_out

namespace board::io::digital_out
{
    void writeLedState(size_t index, ledBrightness_t ledBrightness)
    {
        if (index >= PROJECT_TARGET_MAX_NR_OF_DIGITAL_OUTPUTS)
        {
            return;
        }

        index = detail::map::LED_INDEX(index);

        CORE_MCU_ATOMIC_SECTION
        {
            for (uint8_t i = 0; i < static_cast<int>(ledBrightness_t::B100); i++)
            {
                uint8_t arrayIndex = index / 8;
                uint8_t bit        = index - 8 * arrayIndex;

                core::util::BIT_WRITE(ledState[arrayIndex][i], bit, i < static_cast<int>(ledBrightness) ? 1 : 0);
            }
        }
    }

    size_t rgbFromOutput(size_t index)
    {
        uint8_t row = index / PROJECT_TARGET_NR_OF_LED_COLUMNS;

        uint8_t mod = row % 3;
        row -= mod;

        uint8_t column = index % PROJECT_TARGET_NR_OF_LED_COLUMNS;

        uint8_t result = (row * PROJECT_TARGET_NR_OF_LED_COLUMNS) / 3 + column;

        if (result >= PROJECT_TARGET_NR_OF_RGB_LEDS)
        {
            return PROJECT_TARGET_NR_OF_RGB_LEDS - 1;
        }

        return result;
    }

    size_t rgbComponentFromRgb(size_t index, rgbComponent_t component)
    {
        uint8_t column  = index % PROJECT_TARGET_NR_OF_LED_COLUMNS;
        uint8_t row     = (index / PROJECT_TARGET_NR_OF_LED_COLUMNS) * 3;
        uint8_t address = column + PROJECT_TARGET_NR_OF_LED_COLUMNS * row;

        switch (component)
        {
        case rgbComponent_t::R:
            return address;

        case rgbComponent_t::G:
            return address + PROJECT_TARGET_NR_OF_LED_COLUMNS * 1;

        case rgbComponent_t::B:
            return address + PROJECT_TARGET_NR_OF_LED_COLUMNS * 2;
        }

        return 0;
    }
}    // namespace board::io::digital_out

#endif
#endif