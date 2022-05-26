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
#ifdef DIGITAL_OUTPUT_DRIVER_NATIVE

#include "board/Board.h"
#include "Helpers.h"
#include "board/Internal.h"
#include "core/src/util/Util.h"
#include <Target.h>

using namespace Board::IO::digitalOut;
using namespace Board::detail;
using namespace Board::detail::IO::digitalOut;

namespace
{
    uint8_t                    _pwmCounter;
    core::mcu::io::portWidth_t _portState[NR_OF_DIGITAL_OUTPUT_PORTS][static_cast<uint8_t>(ledBrightness_t::B100)];
}    // namespace

namespace Board::detail::IO::digitalOut
{
    void init()
    {
        for (size_t i = 0; i < NR_OF_DIGITAL_OUTPUTS; i++)
        {
            auto pin = detail::map::ledPin(i);

            CORE_MCU_IO_INIT(CORE_MCU_IO_PIN_PORT(pin),
                             CORE_MCU_IO_PIN_INDEX(pin),
                             core::mcu::io::pinMode_t::OUTPUT_PP,
                             core::mcu::io::pullMode_t::NONE);

            EXT_LED_OFF(CORE_MCU_IO_PIN_PORT(pin), CORE_MCU_IO_PIN_INDEX(pin));
        }
    }

    void update()
    {
        for (size_t port = 0; port < NR_OF_DIGITAL_OUTPUT_PORTS; port++)
        {
            core::mcu::io::portWidth_t updatedPortState = CORE_MCU_IO_READ_OUT_PORT(map::digitalOutPort(port));
            updatedPortState &= detail::map::digitalOutPortClearMask(port);
            updatedPortState |= _portState[port][_pwmCounter];
            CORE_MCU_IO_SET_PORT_STATE(detail::map::digitalOutPort(port), updatedPortState);
        }

        if (++_pwmCounter >= static_cast<uint8_t>(ledBrightness_t::B100))
        {
            _pwmCounter = 0;
        }
    }
}    // namespace Board::detail::IO::digitalOut

namespace Board::IO::digitalOut
{
    void writeLEDstate(size_t index, ledBrightness_t ledBrightness)
    {
        if (index >= NR_OF_DIGITAL_OUTPUTS)
        {
            return;
        }

        index = map::ledIndex(index);

        CORE_MCU_ATOMIC_SECTION
        {
            for (uint8_t i = 0; i < static_cast<int>(ledBrightness_t::B100); i++)
            {
                core::util::BIT_WRITE(_portState[map::ledPortIndex(index)][i], map::ledPinIndex(index), i < static_cast<int>(ledBrightness) ?
#ifndef LED_EXT_INVERT
                                                                                                                                            1
#else
                                                                                                                                            0
#endif
                                                                                                                                            :
#ifndef LED_EXT_INVERT
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

        if (result >= NR_OF_RGB_LEDS)
        {
            return NR_OF_RGB_LEDS - 1;
        }

        return result;
    }

    size_t rgbComponentFromRGB(size_t index, rgbComponent_t component)
    {
        return index * 3 + static_cast<uint8_t>(component);
    }
}    // namespace Board::IO::digitalOut

#endif
#endif