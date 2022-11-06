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

#ifdef HW_SUPPORT_LED_INDICATORS
#ifdef FW_APP

#include "board/Board.h"
#include "board/Internal.h"
#include "core/src/util/Util.h"
#include "core/src/Timing.h"
#include <Target.h>

#include "Indicators.h"

namespace board::detail::io::indicators
{
    void init()
    {
        INIT_ALL_INDICATORS();
    }

    void indicateApplicationLoad()
    {
        static constexpr size_t   TOTAL_FLASHES               = 3;
        static constexpr uint32_t LED_INDICATOR_STARTUP_DELAY = 150;

        for (size_t flash = 0; flash < TOTAL_FLASHES; flash++)
        {
            ALL_INDICATORS_ON();
            core::timing::waitMs(LED_INDICATOR_STARTUP_DELAY);
            ALL_INDICATORS_OFF();
            core::timing::waitMs(LED_INDICATOR_STARTUP_DELAY);
        }
    }

    void update()
    {
        UPDATE_USB_INDICATOR();
        UPDATE_UART_INDICATOR();
        UPDATE_BLE_INDICATOR();
    }
}    // namespace board::detail::io::indicators

namespace board::io::indicators
{
    void indicateTraffic(source_t source, direction_t direction)
    {
        switch (source)
        {
        case source_t::USB:
        {
            INDICATE_USB_TRAFFIC(direction);
        }
        break;

        case source_t::UART:
        {
            INDICATE_UART_TRAFFIC(direction);
        }
        break;

        case source_t::BLE:
        {
            INDICATE_BLE_TRAFFIC(direction);
        }
        break;

        default:
            break;
        }
    }
}    // namespace board::io::indicators

#endif
#endif