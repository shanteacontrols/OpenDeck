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

#include "board/Internal.h"
#include "board/arch/arm/st/Internal.h"
#include "core/src/general/ADC.h"
#include <Target.h>

// stm32f4 specific setup

// 42 constant based on 84MHz system clock and specific prescaler values used here
#define TIMER_US_TO_TICKS(us) (((42 * us) - 1))

namespace core::adc
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
}    // namespace core::adc

namespace Board::detail::setup
{
    void clocks()
    {
        RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
        RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

        /* Configure the main internal regulator output voltage */
        __HAL_RCC_PWR_CLK_ENABLE();
        __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE);

        /* Initializes the CPU, AHB and APB busses clocks */
        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
        RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
        RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
        RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
        RCC_OscInitStruct.PLL.PLLM       = HSE_PLLM;
        RCC_OscInitStruct.PLL.PLLN       = HSE_PLLN;
        RCC_OscInitStruct.PLL.PLLP       = HSE_PLLP;
        RCC_OscInitStruct.PLL.PLLQ       = HSE_PLLQ;

        if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
        {
            Board::detail::errorHandler();
        }

        /* Initializes the CPU, AHB and APB busses clocks */
        RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
        RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
        RCC_ClkInitStruct.AHBCLKDivider  = AHB_CLK_DIV;
        RCC_ClkInitStruct.APB1CLKDivider = APB1_CLK_DIV;
        RCC_ClkInitStruct.APB2CLKDivider = APB2_CLK_DIV;

        if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
        {
            Board::detail::errorHandler();
        }
    }

    void timers()
    {
        TIM_HandleTypeDef       mainTimerHandler  = {};
        TIM_ClockConfigTypeDef  timerClockConfig  = {};
        TIM_MasterConfigTypeDef timerMasterConfig = {};
        TIM_OC_InitTypeDef      timerOCConfig     = {};
        timerOCConfig.OCMode                      = TIM_OCMODE_ACTIVE;
        timerOCConfig.OCPolarity                  = TIM_OCPOLARITY_HIGH;
        timerOCConfig.OCFastMode                  = TIM_OCFAST_DISABLE;

        mainTimerHandler.Instance               = TIM4;
        mainTimerHandler.Init.Prescaler         = 1;
        mainTimerHandler.Init.CounterMode       = TIM_COUNTERMODE_UP;
        mainTimerHandler.Init.Period            = 0xFFFF;
        mainTimerHandler.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
        mainTimerHandler.Init.RepetitionCounter = 0;
        mainTimerHandler.Init.AutoReloadPreload = 0;
        HAL_TIM_Base_Init(&mainTimerHandler);

        timerClockConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
        HAL_TIM_ConfigClockSource(&mainTimerHandler, &timerClockConfig);

        timerMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
        timerMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
        HAL_TIMEx_MasterConfigSynchronization(&mainTimerHandler, &timerMasterConfig);

        timerOCConfig.Pulse = TIMER_US_TO_TICKS(TIMER_PERIOD_MAIN);
        HAL_TIM_OC_ConfigChannel(&mainTimerHandler, &timerOCConfig, TIM_CHANNEL_1);

        timerOCConfig.Pulse = TIMER_US_TO_TICKS(TIMER_PERIOD_PWM);
        HAL_TIM_OC_ConfigChannel(&mainTimerHandler, &timerOCConfig, TIM_CHANNEL_2);

        HAL_TIM_OC_Start_IT(&mainTimerHandler, TIM_CHANNEL_1);
        HAL_TIM_OC_Start_IT(&mainTimerHandler, TIM_CHANNEL_2);
    }

#ifdef ADC_SUPPORTED
    void adc()
    {
        ADC_HandleTypeDef      adcHandler = {};
        ADC_ChannelConfTypeDef sConfig    = {};

        adcHandler.Instance                   = ADC1;
        adcHandler.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;
        adcHandler.Init.Resolution            = ADC_RESOLUTION_12B;
        adcHandler.Init.ScanConvMode          = DISABLE;
        adcHandler.Init.ContinuousConvMode    = DISABLE;
        adcHandler.Init.DiscontinuousConvMode = DISABLE;
        adcHandler.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
        adcHandler.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
        adcHandler.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
        adcHandler.Init.NbrOfConversion       = 1;
        adcHandler.Init.DMAContinuousRequests = DISABLE;
        adcHandler.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
        HAL_ADC_Init(&adcHandler);

        for (int i = 0; i < MAX_ADC_CHANNELS; i++)
        {
            sConfig.Channel      = map::adcChannel(i);
            sConfig.Rank         = 1;
            sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
            HAL_ADC_ConfigChannel(&adcHandler, &sConfig);
        }

        // set first channel
        core::adc::setChannel(map::adcChannel(0));

        HAL_ADC_Start_IT(&adcHandler);
    }
#endif
}    // namespace Board::detail::setup

extern "C" void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
}

extern "C" void HAL_MspDeInit(void)
{
    __HAL_RCC_SYSCFG_CLK_DISABLE();
    __HAL_RCC_PWR_CLK_DISABLE();
}

extern "C" void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1)
    {
        __HAL_RCC_ADC1_CLK_ENABLE();
    }
    else
    {
        return;
    }

    HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ADC_IRQn);
}

extern "C" void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
#ifdef TIM2
    if (htim_base->Instance == TIM2)
    {
        __HAL_RCC_TIM2_CLK_ENABLE();

        HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TIM2_IRQn);
    }
#endif
#ifdef TIM3
    if (htim_base->Instance == TIM3)
    {
        __HAL_RCC_TIM3_CLK_ENABLE();

        HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TIM3_IRQn);
    }
#endif
#ifdef TIM4
    if (htim_base->Instance == TIM4)
    {
        __HAL_RCC_TIM4_CLK_ENABLE();

        HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TIM4_IRQn);
    }
#endif
#ifdef TIM5
    if (htim_base->Instance == TIM5)
    {
        __HAL_RCC_TIM5_CLK_ENABLE();

        HAL_NVIC_SetPriority(TIM5_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TIM5_IRQn);
    }
#endif
#ifdef TIM6
    if (htim_base->Instance == TIM6)
    {
        __HAL_RCC_TIM6_CLK_ENABLE();

        HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
    }
#endif
#ifdef TIM7
    if (htim_base->Instance == TIM7)
    {
        __HAL_RCC_TIM7_CLK_ENABLE();

        HAL_NVIC_SetPriority(TIM7_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TIM7_IRQn);
    }
#endif
#ifdef TIM12
    if (htim_base->Instance == TIM12)
    {
        __HAL_RCC_TIM12_CLK_ENABLE();

        HAL_NVIC_SetPriority(TIM8_BRK_TIM12_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TIM8_BRK_TIM12_IRQn);
    }
#endif
#ifdef TIM12
    if (htim_base->Instance == TIM13)
    {
        __HAL_RCC_TIM13_CLK_ENABLE();

        HAL_NVIC_SetPriority(TIM8_UP_TIM13_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);
    }
#endif
#ifdef TIM14
    if (htim_base->Instance == TIM14)
    {
        __HAL_RCC_TIM14_CLK_ENABLE();

        HAL_NVIC_SetPriority(TIM8_TRG_COM_TIM14_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TIM8_TRG_COM_TIM14_IRQn);
    }
#endif
}

extern "C" void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
    uint8_t channel = 0;

    if (Board::detail::st::uartChannel(huart->Instance, channel))
    {
        auto descriptor = Board::detail::st::uartDescriptor(channel);

        descriptor->enableClock();

        for (size_t i = 0; i < descriptor->pins().size(); i++)
        {
            CORE_IO_INIT(descriptor->pins().at(i));
        }

        if (descriptor->irqn() != 0)
        {
            HAL_NVIC_SetPriority(descriptor->irqn(), 0, 0);
            HAL_NVIC_EnableIRQ(descriptor->irqn());
        }
    }
}

extern "C" void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
    uint8_t channel = 0;

    if (Board::detail::st::uartChannel(huart->Instance, channel))
    {
        auto descriptor = Board::detail::st::uartDescriptor(channel);

        descriptor->disableClock();

        for (size_t i = 0; i < descriptor->pins().size(); i++)
        {
            CORE_IO_DEINIT(descriptor->pins().at(i));
        }

        HAL_NVIC_DisableIRQ(descriptor->irqn());
    }
}

extern "C" void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
    uint8_t channel = 0;

    if (Board::detail::st::i2cChannel(hi2c->Instance, channel))
    {
        auto descriptor = Board::detail::st::i2cDescriptor(channel);

        descriptor->enableClock();

        for (size_t i = 0; i < descriptor->pins().size(); i++)
        {
            CORE_IO_INIT(descriptor->pins().at(i));
        }

        if (descriptor->irqn() != 0)
        {
            HAL_NVIC_SetPriority(descriptor->irqn(), 0, 0);
            HAL_NVIC_EnableIRQ(descriptor->irqn());
        }
    }
}

extern "C" void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
    uint8_t channel = 0;

    if (Board::detail::st::i2cChannel(hi2c->Instance, channel))
    {
        auto descriptor = Board::detail::st::i2cDescriptor(channel);

        descriptor->disableClock();

        for (size_t i = 0; i < descriptor->pins().size(); i++)
        {
            CORE_IO_DEINIT(descriptor->pins().at(i));
        }

        HAL_NVIC_DisableIRQ(descriptor->irqn());
    }
}

extern "C" void InitSystem(void)
{
    // set stack pointer
    __set_MSP(*reinterpret_cast<volatile uint32_t*>(FLASH_START_ADDR));

    // setup FPU
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2)); /* set CP10 and CP11 Full Access */
#endif

    // set the correct address of vector table
    SCB->VTOR = FLASH_START_ADDR;
}