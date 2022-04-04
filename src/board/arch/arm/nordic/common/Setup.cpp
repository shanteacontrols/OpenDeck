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

#include "board/Board.h"
#include "board/Internal.h"
#include "nrf.h"
#include "app_timer.h"

namespace Board::detail::setup
{
    void timers()
    {
        // use systick timer on 100us period
        SysTick_Config(SystemCoreClock / 10000);
        NVIC_EnableIRQ(SysTick_IRQn);

#ifdef FW_APP
        // app timer is used for BLE stack
        BOARD_ERROR_CHECK(app_timer_init(), NRF_SUCCESS);
#endif
    }
}    // namespace Board::detail::setup