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
#include "board/common/io/Helpers.h"
#include "board/common/constants/IO.h"
#include "board/Internal.h"
#include "core/src/util/Util.h"
#include <Target.h>

namespace
{
    uint8_t                    _pwmCounter;
    core::mcu::io::portWidth_t _portState[NR_OF_DIGITAL_OUTPUT_PORTS][static_cast<uint8_t>(Board::IO::ledBrightness_t::B100)];
}    // namespace

namespace Board::detail::IO
{
    void checkDigitalOutputs()
    {
        for (size_t port = 0; port < NR_OF_DIGITAL_OUTPUT_PORTS; port++)
        {
            core::mcu::io::portWidth_t updatedPortState = CORE_MCU_IO_READ_OUT_PORT(detail::map::digitalOutPort(port));
            updatedPortState &= detail::map::digitalOutPortClearMask(port);
            updatedPortState |= _portState[port][_pwmCounter];
            CORE_MCU_IO_SET_PORT_STATE(detail::map::digitalOutPort(port), updatedPortState);
        }

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
                core::util::BIT_WRITE(_portState[detail::map::ledPortIndex(ledID)][i], detail::map::ledPinIndex(ledID), i < static_cast<int>(ledBrightness) ?
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