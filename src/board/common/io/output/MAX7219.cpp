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
#ifdef DIGITAL_OUTPUT_DRIVER_MAX7219

#include "board/Board.h"
#include "board/common/io/Helpers.h"
#include "board/common/constants/IO.h"
#include "board/Internal.h"
#include "core/src/util/Util.h"
#include <Target.h>

using namespace Board::IO::digitalOut;
using namespace Board::detail;
using namespace Board::detail::IO::digitalOut;

namespace
{
    constexpr uint8_t MAX7219_REG_DECODEMODE  = 0x9;
    constexpr uint8_t MAX7219_REG_INTENSITY   = 0xA;
    constexpr uint8_t MAX7219_REG_SCANLIMIT   = 0xB;
    constexpr uint8_t MAX7219_REG_SHUTDOWN    = 0xC;
    constexpr uint8_t MAX7219_REG_DISPLAYTEST = 0xF;

    uint8_t _columns[8];

    void sendCommand(uint8_t reg, uint8_t data)
    {
        CORE_MCU_ATOMIC_SECTION
        {
            CORE_MCU_IO_SET_LOW(MAX7219_LATCH_PORT, MAX7219_LATCH_PIN);

            auto sendByte = [](uint8_t byte)
            {
                for (int8_t bit = 7; bit >= 0; bit--)
                {
                    core::util::BIT_READ(byte, bit) ? CORE_MCU_IO_SET_HIGH(MAX7219_DATA_PORT, MAX7219_DATA_PIN) : CORE_MCU_IO_SET_LOW(MAX7219_DATA_PORT, MAX7219_DATA_PIN);
                    CORE_MCU_IO_SET_LOW(MAX7219_CLK_PORT, MAX7219_CLK_PIN);
                    Board::detail::IO::spiWait();
                    CORE_MCU_IO_SET_HIGH(MAX7219_CLK_PORT, MAX7219_CLK_PIN);
                }
            };

            sendByte(reg);
            sendByte(data);

            CORE_MCU_IO_SET_HIGH(MAX7219_LATCH_PORT, MAX7219_LATCH_PIN);
        }
    }
}    // namespace

namespace Board::detail::IO::digitalOut
{
    void init()
    {
        CORE_MCU_IO_INIT(MAX7219_DATA_PORT, MAX7219_DATA_PIN, core::mcu::io::pinMode_t::OUTPUT_PP, core::mcu::io::pullMode_t::NONE);
        CORE_MCU_IO_INIT(MAX7219_CLK_PORT, MAX7219_CLK_PIN, core::mcu::io::pinMode_t::OUTPUT_PP, core::mcu::io::pullMode_t::NONE);
        CORE_MCU_IO_INIT(MAX7219_LATCH_PORT, MAX7219_LATCH_PIN, core::mcu::io::pinMode_t::OUTPUT_PP, core::mcu::io::pullMode_t::NONE);

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
}    // namespace Board::detail::IO::digitalOut

namespace Board::IO::digitalOut
{
    void writeLEDstate(size_t index, ledBrightness_t ledBrightness)
    {
        index          = map::ledIndex(index);
        uint8_t column = index % 8;
        uint8_t row    = index / 8;

        core::util::BIT_WRITE(_columns[column], row, ledBrightness != ledBrightness_t::OFF);

        // update only the changed column
        sendCommand(column + 1, _columns[column]);
    }

    size_t rgbFromOutput(size_t index)
    {
        uint8_t row = index / 8;

        uint8_t mod = row % 3;
        row -= mod;

        uint8_t column = index % 8;

        uint8_t result = (row * 8) / 3 + column;

        if (result >= NR_OF_RGB_LEDS)
        {
            return NR_OF_RGB_LEDS - 1;
        }

        return result;
    }

    size_t rgbComponentFromRGB(size_t index, rgbComponent_t component)
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
}    // namespace Board::IO::digitalOut

#endif
#endif