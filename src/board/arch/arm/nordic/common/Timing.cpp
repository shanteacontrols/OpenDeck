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

#include <MCU.h>

// in microseconds
#define SYS_TICK_INTERVAL 100

#if TIMER_PERIOD_MAIN % SYS_TICK_INTERVAL
#error TIMER_PERIOD_MAIN must be multiple of 100
#endif

#if TIMER_PERIOD_PWM % SYS_TICK_INTERVAL
#error TIMER_PERIOD_PWM must be multiple of 100
#endif

namespace
{
    constexpr uint32_t TIMER_PERIOD_MAIN_CNT = TIMER_PERIOD_MAIN / SYS_TICK_INTERVAL;
    constexpr uint32_t TIMER_PERIOD_PWM_CNT  = TIMER_PERIOD_PWM / SYS_TICK_INTERVAL;
}    // namespace

extern "C" void SysTick_Handler(void)
{
    static uint32_t cnt = 0;

    cnt++;

    if ((cnt % TIMER_PERIOD_MAIN_CNT) == 0)
    {
        Board::detail::isrHandling::timer(TIMER_CHANNEL_MAIN);
    }

    if ((cnt % TIMER_PERIOD_PWM_CNT) == 0)
    {
        Board::detail::isrHandling::timer(TIMER_CHANNEL_PWM);
    }
}