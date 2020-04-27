/*

Copyright 2015-2020 Igor Petrovic

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
#include "Pins.h"
#include "board/Internal.h"
#include "board/common/io/Helpers.h"
#include "core/src/general/IO.h"
#include "core/src/general/Atomic.h"
#include "core/src/general/ADC.h"
#include "core/src/general/Timing.h"

namespace
{
    TIM_HandleTypeDef htim7;
    ADC_HandleTypeDef hadc1;
}    // namespace

//UART3 on this board maps to UART channel 0 in application

#ifdef FW_APP
//not needed in bootloader
extern "C" void USART1_IRQHandler(void)
{
    Board::detail::isrHandling::uart(0);
}

extern "C" void ADC_IRQHandler(void)
{
    Board::detail::isrHandling::adc(hadc1.Instance->DR);
}
#endif

extern "C" void TIM7_IRQHandler(void)
{
    __HAL_TIM_CLEAR_IT(&htim7, TIM_IT_UPDATE);
    Board::detail::isrHandling::mainTimer();
}

namespace Board
{
    namespace detail
    {
        namespace setup
        {
            void clocks()
            {
                RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
                RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

                /* Configure the main internal regulator output voltage */
                __HAL_RCC_PWR_CLK_ENABLE();
                __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

                /* Initializes the CPU, AHB and APB busses clocks */
                RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
                RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
                RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
                RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
                RCC_OscInitStruct.PLL.PLLM       = 8;
                RCC_OscInitStruct.PLL.PLLN       = 168;
                RCC_OscInitStruct.PLL.PLLP       = RCC_PLLP_DIV2;
                RCC_OscInitStruct.PLL.PLLQ       = 7;

                if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
                    Board::detail::errorHandler();

                /* Initializes the CPU, AHB and APB busses clocks */
                RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
                RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
                RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV2;
                RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
                RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

                if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
                    Board::detail::errorHandler();
            }

            void io()
            {
                CORE_IO_CONFIG({ DI_1_PORT, DI_1_PIN, core::io::pinMode_t::input, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_2_PORT, DI_2_PIN, core::io::pinMode_t::input, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_3_PORT, DI_3_PIN, core::io::pinMode_t::input, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_4_PORT, DI_4_PIN, core::io::pinMode_t::input, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_5_PORT, DI_5_PIN, core::io::pinMode_t::input, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_6_PORT, DI_6_PIN, core::io::pinMode_t::input, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_7_PORT, DI_7_PIN, core::io::pinMode_t::input, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_8_PORT, DI_8_PIN, core::io::pinMode_t::input, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_9_PORT, DI_9_PIN, core::io::pinMode_t::input, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_10_PORT, DI_10_PIN, core::io::pinMode_t::input, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_11_PORT, DI_11_PIN, core::io::pinMode_t::input, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_12_PORT, DI_12_PIN, core::io::pinMode_t::input, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_13_PORT, DI_13_PIN, core::io::pinMode_t::input, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_14_PORT, DI_14_PIN, core::io::pinMode_t::input, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });

                CORE_IO_CONFIG({ MUX_1_IN_PORT, MUX_1_IN_PIN, core::io::pinMode_t::analog, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ MUX_2_IN_PORT, MUX_2_IN_PIN, core::io::pinMode_t::analog, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });

                CORE_IO_CONFIG({ BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN, core::io::pinMode_t::input, core::io::pullMode_t::up, core::io::gpioSpeed_t::medium, 0x00 });

                CORE_IO_CONFIG({ MUX_S0_PORT, MUX_S0_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ MUX_S1_PORT, MUX_S1_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ MUX_S2_PORT, MUX_S2_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ MUX_S3_PORT, MUX_S3_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
            }

            void adc()
            {
                ADC_ChannelConfTypeDef sConfig = { 0 };

                hadc1.Instance                   = ADC1;
                hadc1.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV2;
                hadc1.Init.Resolution            = ADC_RESOLUTION_12B;
                hadc1.Init.ScanConvMode          = DISABLE;
                hadc1.Init.ContinuousConvMode    = DISABLE;
                hadc1.Init.DiscontinuousConvMode = DISABLE;
                hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
                hadc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
                hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
                hadc1.Init.NbrOfConversion       = 1;
                hadc1.Init.DMAContinuousRequests = DISABLE;
                hadc1.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
                HAL_ADC_Init(&hadc1);

                for (int i = 0; i < NUMBER_OF_MUX; i++)
                {
                    sConfig.Channel      = map::adcChannel(i);
                    sConfig.Rank         = 1;
                    sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
                    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
                }

                //set first channel
                core::adc::setChannel(map::adcChannel(0));

                HAL_ADC_Start_IT(&hadc1);
            }

            void timers()
            {
                htim7.Instance               = TIM7;
                htim7.Init.Prescaler         = 0;
                htim7.Init.CounterMode       = TIM_COUNTERMODE_UP;
                htim7.Init.Period            = 41999;
                htim7.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
                htim7.Init.RepetitionCounter = 0;
                htim7.Init.AutoReloadPreload = 0;
                HAL_TIM_Base_Init(&htim7);

                HAL_TIM_Base_Start_IT(&htim7);
            }
        }    // namespace setup
    }        // namespace detail
}    // namespace Board