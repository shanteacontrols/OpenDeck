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

#ifdef PROJECT_TARGET_NR_OF_UNUSED_IO

#include "board/board.h"
#include "internal.h"
#include <target.h>

#include "core/util/util.h"

using namespace board::detail;

namespace board::detail::io::unused
{
    void init()
    {
        for (size_t i = 0; i < PROJECT_TARGET_NR_OF_UNUSED_IO; i++)
        {
            auto unusedPin = map::UNUSED_PIN(i);

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
}    // namespace board::detail::io::unused

#endif