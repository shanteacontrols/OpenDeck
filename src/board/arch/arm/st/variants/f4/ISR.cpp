/*

Copyright 2015-2021 Igor Petrovic

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

#include "board/Internal.h"
#include "core/src/general/ADC.h"
#include <MCU.h>

// stm32f4 specific ISRs

namespace core
{
#ifdef ADC_SUPPORTED
    namespace adc
    {
        void startConversion()
        {
            /* Clear regular group conversion flag and overrun flag */
            /* (To ensure of no unknown state from potential previous ADC operations) */
            ADC1->SR = ~(ADC_FLAG_EOC | ADC_FLAG_OVR);

            /* Enable end of conversion interrupt for regular group */
            ADC1->CR1 |= (ADC_IT_EOC | ADC_IT_OVR);

            /* Enable the selected ADC software conversion for regular group */
            ADC1->CR2 |= (uint32_t)ADC_CR2_SWSTART;
        }

        void setChannel(uint32_t adcChannel)
        {
            /* Clear the old SQx bits for the selected rank */
            ADC1->SQR3 &= ~ADC_SQR3_RK(ADC_SQR3_SQ1, 1);

            /* Set the SQx bits for the selected rank */
            ADC1->SQR3 |= ADC_SQR3_RK(adcChannel, 1);
        }
    }    // namespace adc
#endif
}    // namespace core