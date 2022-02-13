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
#include <MCU.h>

extern "C" void TIM4_IRQHandler(void)
{
    // To avoid having global variables or adding more internal APIs,
    // store channel count in static variables the first time
    // interrupt for specific channel is triggered.
    // Needed since CCRx register needs to be updated on each compare
    // match.

    static int32_t ch1Cnt = -1;
    static int32_t ch2Cnt = -1;

    if ((TIM4->SR & TIM_IT_CC1) != RESET)
    {
        if (ch1Cnt == -1)
        {
            ch1Cnt = TIM4->CCR1;
        }

        Board::detail::isrHandling::timer(0);

        TIM4->CCR1 += ch1Cnt;
        TIM4->SR &= ~TIM_IT_CC1;
    }
    else if ((TIM4->SR & TIM_IT_CC2) != RESET)
    {
        if (ch2Cnt == -1)
        {
            ch2Cnt = TIM4->CCR2;
        }

        Board::detail::isrHandling::timer(1);

        TIM4->CCR2 += ch2Cnt;
        TIM4->SR &= ~TIM_IT_CC2;
    }
}