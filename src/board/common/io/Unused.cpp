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

#ifdef TOTAL_UNUSED_IO

#include "board/Board.h"
#include "board/Internal.h"
#include "core/src/util/Util.h"
#include <Target.h>

using namespace Board::detail;

namespace Board::detail::IO::unused
{
    void init()
    {
        for (size_t i = 0; i < TOTAL_UNUSED_IO; i++)
        {
            auto unusedPin = map::unusedPin(i);

            CORE_MCU_IO_INIT(unusedPin.pin.port, unusedPin.pin.index, unusedPin.pin.mode);

            // for input mode, pull up is activated so no need to set state via CORE_IO_SET_STATE
            if (unusedPin.pin.mode == core::mcu::io::pinMode_t::OUTPUT_PP)
            {
                CORE_MCU_IO_SET_STATE(unusedPin.pin.port,
                                      unusedPin.pin.index,
                                      unusedPin.state);
            }
        }
    }
}    // namespace Board::detail::IO::unused

#endif