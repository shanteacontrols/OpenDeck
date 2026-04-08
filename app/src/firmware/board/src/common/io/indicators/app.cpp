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

#ifdef PROJECT_TARGET_SUPPORT_LED_INDICATORS
#ifdef OPENDECK_FW_APP

#include "indicators.h"
#include "board/board.h"
#include "internal.h"
#include <target.h>

#include "core/util/util.h"

namespace
{
    bool              factoryResetInProgress;
    volatile uint16_t factoryResetIndicatorTimeout;
    bool              factoryResetIndicatorState;
}    // namespace

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
            core::mcu::timing::waitMs(LED_INDICATOR_STARTUP_DELAY);
            ALL_INDICATORS_OFF();
            core::mcu::timing::waitMs(LED_INDICATOR_STARTUP_DELAY);
        }
    }

    void update()
    {
        using namespace board::io::indicators;

        if (!factoryResetInProgress)
        {
            UPDATE_USB_INDICATOR();
            UPDATE_UART_INDICATOR();
            UPDATE_BLE_INDICATOR();
        }
        else
        {
            if (++factoryResetIndicatorTimeout == LED_FACTORY_RESET_INDICATOR_TIMEOUT)
            {
                factoryResetIndicatorTimeout = 0;
                factoryResetIndicatorState   = !factoryResetIndicatorState;

                if (factoryResetIndicatorState)
                {
                    IN_INDICATORS_ON();
                    OUT_INDICATORS_OFF();
                }
                else
                {
                    OUT_INDICATORS_ON();
                    IN_INDICATORS_OFF();
                }
            }
        }
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

    void indicateFactoryReset()
    {
        ALL_INDICATORS_OFF();
        factoryResetInProgress = true;
    }
}    // namespace board::io::indicators

#endif
#endif