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

#include "stm32f4xx_hal.h"

//PLL values for 84MHz clock
#if (HSE_VALUE == 8000000)
#define HSE_PLLM     4
#define HSE_PLLN     168
#define HSE_PLLP     RCC_PLLP_DIV4
#define HSE_PLLQ     7
#define AHB_CLK_DIV  RCC_SYSCLK_DIV1
#define APB1_CLK_DIV RCC_SYSCLK_DIV2
#define APB2_CLK_DIV RCC_SYSCLK_DIV1
#elif (HSE_VALUE == 16000000)
#define HSE_PLLM     8
#define HSE_PLLN     168
#define HSE_PLLP     RCC_PLLP_DIV4
#define HSE_PLLQ     7
#define AHB_CLK_DIV  RCC_SYSCLK_DIV1
#define APB1_CLK_DIV RCC_SYSCLK_DIV2
#define APB2_CLK_DIV RCC_SYSCLK_DIV1
#elif (HSE_VALUE == 25000000)
#define HSE_PLLM     25
#define HSE_PLLN     336
#define HSE_PLLP     RCC_PLLP_DIV4
#define HSE_PLLQ     7
#define AHB_CLK_DIV  RCC_SYSCLK_DIV1
#define APB1_CLK_DIV RCC_SYSCLK_DIV2
#define APB2_CLK_DIV RCC_SYSCLK_DIV1
#else
#error Invalid clock value
#endif

#define PWR_REGULATOR_VOLTAGE_SCALE PWR_REGULATOR_VOLTAGE_SCALE2