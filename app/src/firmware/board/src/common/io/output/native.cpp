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
#ifdef PROJECT_TARGET_DRIVER_DIGITAL_OUTPUT_NATIVE

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
    uint8_t                    pwmCounter;
    core::mcu::io::portWidth_t portState[PROJECT_TARGET_NR_OF_DIGITAL_OUTPUT_PORTS][static_cast<uint8_t>(ledBrightness_t::B100)];
}    // namespace

namespace board::detail::io::digital_out
{
    void init()
    {
        for (size_t i = 0; i < PROJECT_TARGET_MAX_NR_OF_DIGITAL_OUTPUTS; i++)
        {
            auto pin = detail::map::LED_PIN(i);

            CORE_MCU_IO_INIT(pin.port,
                             pin.index,
                             core::mcu::io::pinMode_t::OUTPUT_PP,
                             core::mcu::io::pullMode_t::NONE);

            EXT_LED_OFF(pin.port, pin.index);
        }
    }

    void update()
    {
        for (size_t port = 0; port < PROJECT_TARGET_NR_OF_DIGITAL_OUTPUT_PORTS; port++)
        {
            core::mcu::io::portWidth_t updatedPortState = CORE_MCU_IO_READ_OUT_PORT(map::DIGITAL_OUT_PORT(port));
            updatedPortState &= detail::map::DIGITAL_OUT_PORT_CLEAR_MASK(port);
            updatedPortState |= portState[port][pwmCounter];
            CORE_MCU_IO_SET_PORT_STATE(detail::map::DIGITAL_OUT_PORT(port), updatedPortState);
        }

        if (++pwmCounter >= static_cast<uint8_t>(ledBrightness_t::B100))
        {
            pwmCounter = 0;
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

        index = map::LED_INDEX(index);

        CORE_MCU_ATOMIC_SECTION
        {
            for (uint8_t i = 0; i < static_cast<int>(ledBrightness_t::B100); i++)
            {
                core::util::BIT_WRITE(portState[map::LED_PORT_INDEX(index)][i], map::LED_PIN_INDEX(index), i < static_cast<int>(ledBrightness) ?
#ifndef PROJECT_TARGET_LEDS_EXT_INVERT
                                                                                                                                               1
#else
                                                                                                                                               0
#endif
                                                                                                                                               :
#ifndef PROJECT_TARGET_LEDS_EXT_INVERT
                                                                                                                                               0
#else
                                                                                                                                               1
#endif

                );
            }
        }
    }

    size_t rgbFromOutput(size_t index)
    {
        uint8_t result = index / 3;

        if (result >= PROJECT_TARGET_NR_OF_RGB_LEDS)
        {
            return PROJECT_TARGET_NR_OF_RGB_LEDS - 1;
        }

        return result;
    }

    size_t rgbComponentFromRgb(size_t index, rgbComponent_t component)
    {
        return index * 3 + static_cast<uint8_t>(component);
    }
}    // namespace board::io::digital_out

#endif
#endif