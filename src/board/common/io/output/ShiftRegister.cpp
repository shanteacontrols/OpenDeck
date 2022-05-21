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
#ifdef DIGITAL_OUTPUT_DRIVER_SHIFT_REGISTER

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
}    // namespace

namespace Board::detail::IO
{
    void checkDigitalOutputs()
    {
        CORE_MCU_IO_SET_LOW(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);

        for (int j = 0; j < NUMBER_OF_OUT_SR; j++)
        {
            for (int i = 0; i < 8; i++)
            {
                size_t  ledID      = i + j * 8;
                uint8_t arrayIndex = ledID / 8;
                uint8_t ledBit     = ledID - 8 * arrayIndex;

                core::util::BIT_READ(_ledState[arrayIndex][_pwmCounter], ledBit) ? EXT_LED_ON(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN) : EXT_LED_OFF(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN);
                CORE_MCU_IO_SET_LOW(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
                detail::IO::sr595wait();
                CORE_MCU_IO_SET_HIGH(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
            }
        }

        CORE_MCU_IO_SET_HIGH(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);

        if (++_pwmCounter >= static_cast<uint8_t>(Board::IO::ledBrightness_t::B100))
        {
            _pwmCounter = 0;
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
        return rgbID * 3 + static_cast<uint8_t>(index);
    }

    size_t rgbIndex(size_t ledID)
    {
        uint8_t result = ledID / 3;

        if (result >= Board::detail::IO::NR_OF_RGB_LEDS)
        {
            return Board::detail::IO::NR_OF_RGB_LEDS - 1;
        }

        return result;
    }
}    // namespace Board::IO

#endif
#endif