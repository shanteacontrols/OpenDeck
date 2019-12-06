/*

Copyright 2015-2019 Igor Petrovic

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

extern "C" void USART3_IRQHandler(void)
{
    Board::detail::isrHandling::uart(0);
}

extern "C" void TIM7_IRQHandler(void)
{
    __HAL_TIM_CLEAR_IT(&htim7, TIM_IT_UPDATE);
    Board::detail::isrHandling::mainTimer();
}

extern "C" void ADC_IRQHandler(void)
{
    HAL_ADC_IRQHandler(&hadc1);
}

namespace Board
{
    namespace detail
    {
        namespace setup
        {
            void clocks()
            {
                //system clock config - use 168MHz
                RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
                RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

                /* Configure the main internal regulator output voltage */
                __HAL_RCC_PWR_CLK_ENABLE();
                __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

                /* Initializes the CPU, AHB and APB busses clocks */
                RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
                RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
                RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
                RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
                RCC_OscInitStruct.PLL.PLLM = 4;
                RCC_OscInitStruct.PLL.PLLN = 168;
                RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
                RCC_OscInitStruct.PLL.PLLQ = 7;

                HAL_RCC_OscConfig(&RCC_OscInitStruct);

                /* Initializes the CPU, AHB and APB busses clocks */
                RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
                RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
                RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
                RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
                RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

                HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
            }

            void io()
            {
                CORE_IO_CONFIG({ DI_1_PORT, DI_1_PIN, core::io::pinMode_t::input, core::io::pullMode_t::up, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_2_PORT, DI_2_PIN, core::io::pinMode_t::input, core::io::pullMode_t::up, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_3_PORT, DI_3_PIN, core::io::pinMode_t::input, core::io::pullMode_t::up, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_4_PORT, DI_4_PIN, core::io::pinMode_t::input, core::io::pullMode_t::up, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_5_PORT, DI_5_PIN, core::io::pinMode_t::input, core::io::pullMode_t::up, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_6_PORT, DI_6_PIN, core::io::pinMode_t::input, core::io::pullMode_t::up, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_7_PORT, DI_7_PIN, core::io::pinMode_t::input, core::io::pullMode_t::up, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_8_PORT, DI_8_PIN, core::io::pinMode_t::input, core::io::pullMode_t::up, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_9_PORT, DI_9_PIN, core::io::pinMode_t::input, core::io::pullMode_t::up, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_10_PORT, DI_10_PIN, core::io::pinMode_t::input, core::io::pullMode_t::up, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_11_PORT, DI_11_PIN, core::io::pinMode_t::input, core::io::pullMode_t::up, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_12_PORT, DI_12_PIN, core::io::pinMode_t::input, core::io::pullMode_t::up, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_13_PORT, DI_13_PIN, core::io::pinMode_t::input, core::io::pullMode_t::up, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_14_PORT, DI_14_PIN, core::io::pinMode_t::input, core::io::pullMode_t::up, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_15_PORT, DI_15_PIN, core::io::pinMode_t::input, core::io::pullMode_t::up, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_16_PORT, DI_16_PIN, core::io::pinMode_t::input, core::io::pullMode_t::up, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_17_PORT, DI_17_PIN, core::io::pinMode_t::input, core::io::pullMode_t::up, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DI_18_PORT, DI_18_PIN, core::io::pinMode_t::input, core::io::pullMode_t::up, core::io::gpioSpeed_t::medium, 0x00 });

                CORE_IO_CONFIG({ DO_1_PORT, DO_1_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                EXT_LED_OFF(DO_1_PORT, DO_1_PIN);

                CORE_IO_CONFIG({ DO_2_PORT, DO_2_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                EXT_LED_OFF(DO_2_PORT, DO_2_PIN);

                CORE_IO_CONFIG({ DO_3_PORT, DO_3_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                EXT_LED_OFF(DO_3_PORT, DO_3_PIN);

                CORE_IO_CONFIG({ DO_4_PORT, DO_4_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                EXT_LED_OFF(DO_4_PORT, DO_4_PIN);

                CORE_IO_CONFIG({ DO_5_PORT, DO_5_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                EXT_LED_OFF(DO_5_PORT, DO_5_PIN);

                CORE_IO_CONFIG({ DO_6_PORT, DO_6_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                EXT_LED_OFF(DO_6_PORT, DO_6_PIN);

                CORE_IO_CONFIG({ DO_7_PORT, DO_7_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                EXT_LED_OFF(DO_7_PORT, DO_7_PIN);

                CORE_IO_CONFIG({ DO_8_PORT, DO_8_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                EXT_LED_OFF(DO_8_PORT, DO_8_PIN);

                CORE_IO_CONFIG({ DO_9_PORT, DO_9_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                EXT_LED_OFF(DO_9_PORT, DO_9_PIN);

                CORE_IO_CONFIG({ DO_10_PORT, DO_10_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                EXT_LED_OFF(DO_10_PORT, DO_10_PIN);

                CORE_IO_CONFIG({ DO_11_PORT, DO_11_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                EXT_LED_OFF(DO_11_PORT, DO_11_PIN);

                CORE_IO_CONFIG({ DO_12_PORT, DO_12_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                EXT_LED_OFF(DO_12_PORT, DO_12_PIN);

                CORE_IO_CONFIG({ DO_13_PORT, DO_13_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                EXT_LED_OFF(DO_13_PORT, DO_13_PIN);

                CORE_IO_CONFIG({ DO_14_PORT, DO_14_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                EXT_LED_OFF(DO_14_PORT, DO_14_PIN);

                CORE_IO_CONFIG({ DO_15_PORT, DO_15_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                EXT_LED_OFF(DO_15_PORT, DO_15_PIN);

                CORE_IO_CONFIG({ DO_16_PORT, DO_16_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                EXT_LED_OFF(DO_16_PORT, DO_16_PIN);

                CORE_IO_CONFIG({ AI_1_PORT, AI_1_PIN, core::io::pinMode_t::analog, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_SET_LOW(AI_1_PORT, AI_1_PIN);

                CORE_IO_CONFIG({ AI_2_PORT, AI_2_PIN, core::io::pinMode_t::analog, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_SET_LOW(AI_2_PORT, AI_2_PIN);

                CORE_IO_CONFIG({ AI_3_PORT, AI_3_PIN, core::io::pinMode_t::analog, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_SET_LOW(AI_3_PORT, AI_3_PIN);

                CORE_IO_CONFIG({ AI_4_PORT, AI_4_PIN, core::io::pinMode_t::analog, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_SET_LOW(AI_4_PORT, AI_4_PIN);

                CORE_IO_CONFIG({ AI_5_PORT, AI_5_PIN, core::io::pinMode_t::analog, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_SET_LOW(AI_5_PORT, AI_5_PIN);

                CORE_IO_CONFIG({ AI_6_PORT, AI_6_PIN, core::io::pinMode_t::analog, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_SET_LOW(AI_6_PORT, AI_6_PIN);

                CORE_IO_CONFIG({ AI_7_PORT, AI_7_PIN, core::io::pinMode_t::analog, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_SET_LOW(AI_7_PORT, AI_7_PIN);

                CORE_IO_CONFIG({ AI_8_PORT, AI_8_PIN, core::io::pinMode_t::analog, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_SET_LOW(AI_8_PORT, AI_8_PIN);

                CORE_IO_CONFIG({ LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                INT_LED_OFF(LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN);

                CORE_IO_CONFIG({ LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                INT_LED_OFF(LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN);

                CORE_IO_CONFIG({ LED_MIDI_IN_USB_PORT, LED_MIDI_IN_USB_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                INT_LED_OFF(LED_MIDI_IN_USB_PORT, LED_MIDI_IN_USB_PIN);

                CORE_IO_CONFIG({ LED_MIDI_OUT_USB_PORT, LED_MIDI_OUT_USB_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                INT_LED_OFF(LED_MIDI_OUT_USB_PORT, LED_MIDI_OUT_USB_PIN);

                //uart setup
                CORE_IO_CONFIG({ UART_0_RX_PORT, UART_0_RX_PIN, core::io::pinMode_t::alternatePP, core::io::pullMode_t::up, core::io::gpioSpeed_t::veryHigh, GPIO_AF7_USART3 });
                CORE_IO_CONFIG({ UART_0_TX_PORT, UART_0_TX_PIN, core::io::pinMode_t::alternatePP, core::io::pullMode_t::up, core::io::gpioSpeed_t::veryHigh, GPIO_AF7_USART3 });
            }

            void adc()
            {
                ADC_ChannelConfTypeDef sConfig = { 0 };

                hadc1.Instance = ADC1;
                hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
                hadc1.Init.Resolution = ADC_RESOLUTION_12B;
                hadc1.Init.ScanConvMode = DISABLE;
                hadc1.Init.ContinuousConvMode = DISABLE;
                hadc1.Init.DiscontinuousConvMode = DISABLE;
                hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
                hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
                hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
                hadc1.Init.NbrOfConversion = 1;
                hadc1.Init.DMAContinuousRequests = DISABLE;
                hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
                HAL_ADC_Init(&hadc1);

                sConfig.Channel = map::adcChannel(0);
                sConfig.Rank = 1;
                sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
                HAL_ADC_ConfigChannel(&hadc1, &sConfig);

                HAL_ADC_Start_IT(&hadc1);
            }

            void timers()
            {
                TIM_MasterConfigTypeDef sMasterConfig = { 0 };

                htim7.Instance = TIM7;
                htim7.Init.Prescaler = 0;
                htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
                htim7.Init.Period = 0;
                htim7.Init.AutoReloadPreload = 41999;
                HAL_TIM_Base_Init(&htim7);

                sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
                sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
                HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig);

                HAL_TIM_Base_Start_IT(&htim7);
            }
        }    // namespace setup
    }        // namespace detail
}    // namespace Board