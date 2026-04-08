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
#ifdef PROJECT_TARGET_DRIVER_DIGITAL_OUTPUT_MAX7219

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
    constexpr uint8_t MAX7219_REG_DECODEMODE  = 0x9;
    constexpr uint8_t MAX7219_REG_INTENSITY   = 0xA;
    constexpr uint8_t MAX7219_REG_SCANLIMIT   = 0xB;
    constexpr uint8_t MAX7219_REG_SHUTDOWN    = 0xC;
    constexpr uint8_t MAX7219_REG_DISPLAYTEST = 0xF;
    uint8_t           columns[8];

    void sendCommand(uint8_t reg, uint8_t data)
    {
        CORE_MCU_ATOMIC_SECTION
        {
            CORE_MCU_IO_SET_LOW(PIN_PORT_MAX7219_LATCH, PIN_INDEX_MAX7219_LATCH);

            auto sendByte = [](uint8_t byte)
            {
                for (int8_t bit = 7; bit >= 0; bit--)
                {
                    CORE_MCU_IO_SET_STATE(PIN_PORT_MAX7219_DATA,
                                          PIN_INDEX_MAX7219_DATA,
                                          core::util::BIT_READ(byte, bit));

                    CORE_MCU_IO_SET_LOW(PIN_PORT_MAX7219_CLK, PIN_INDEX_MAX7219_CLK);
                    board::detail::io::spiWait();
                    CORE_MCU_IO_SET_HIGH(PIN_PORT_MAX7219_CLK, PIN_INDEX_MAX7219_CLK);
                }
            };

            sendByte(reg);
            sendByte(data);

            CORE_MCU_IO_SET_HIGH(PIN_PORT_MAX7219_LATCH, PIN_INDEX_MAX7219_LATCH);
        }
    }
}    // namespace

namespace board::detail::io::digital_out
{
    void init()
    {
        CORE_MCU_IO_INIT(PIN_PORT_MAX7219_DATA,
                         PIN_INDEX_MAX7219_DATA,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);

        CORE_MCU_IO_INIT(PIN_PORT_MAX7219_CLK,
                         PIN_INDEX_MAX7219_CLK,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);

        CORE_MCU_IO_INIT(PIN_PORT_MAX7219_LATCH,
                         PIN_INDEX_MAX7219_LATCH,
                         core::mcu::io::pinMode_t::OUTPUT_PP,
                         core::mcu::io::pullMode_t::NONE);

        // no display testing
        sendCommand(MAX7219_REG_DISPLAYTEST, 0);

        // show all 8 digits
        sendCommand(MAX7219_REG_SCANLIMIT, 7);

        // use led matrix (no digits)
        sendCommand(MAX7219_REG_DECODEMODE, 0);

        // clear display
        for (uint8_t col = 0; col < 8; col++)
        {
            sendCommand(col + 1, 0);
        }

        // use 2/3rd of possible intensity
        sendCommand(MAX7219_REG_INTENSITY, 10);

        // normal operation - start up
        sendCommand(MAX7219_REG_SHUTDOWN, 1);
    }
}    // namespace board::detail::io::digital_out

namespace board::io::digital_out
{
    void writeLedState(size_t index, ledBrightness_t ledBrightness)
    {
        index          = map::LED_INDEX(index);
        uint8_t column = index % 8;
        uint8_t row    = index / 8;

        core::util::BIT_WRITE(columns[column], row, ledBrightness != ledBrightness_t::OFF);

        // update only the changed column
        sendCommand(column + 1, columns[column]);
    }

    size_t rgbFromOutput(size_t index)
    {
        uint8_t row = index / 8;

        uint8_t mod = row % 3;
        row -= mod;

        uint8_t column = index % 8;

        uint8_t result = (row * 8) / 3 + column;

        if (result >= PROJECT_TARGET_NR_OF_RGB_LEDS)
        {
            return PROJECT_TARGET_NR_OF_RGB_LEDS - 1;
        }

        return result;
    }

    size_t rgbComponentFromRgb(size_t index, rgbComponent_t component)
    {
        uint8_t column  = index % 8;
        uint8_t row     = (index / 8) * 3;
        uint8_t address = column + 8 * row;

        switch (component)
        {
        case rgbComponent_t::R:
            return address;

        case rgbComponent_t::G:
            return address + 8 * 1;

        case rgbComponent_t::B:
            return address + 8 * 2;
        }

        return 0;
    }
}    // namespace board::io::digital_out

#endif
#endif