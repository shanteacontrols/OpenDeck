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

#include "board/Board.h"
#include "board/Internal.h"
#include "stm32f4xx_hal.h"
#include "core/src/general/Timing.h"
#ifdef FW_CDC
#include "board/common/comm/usb/descriptors/cdc/Descriptors.h"
#endif
#include "MCU.h"

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

//This function handles USB On The Go FS global interrupt.
extern "C" void OTG_FS_IRQHandler(void)
{
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}

//This function handles Non maskable interrupt.
extern "C" void NMI_Handler(void)
{
}

//This function handles Hard fault interrupt.
extern "C" void HardFault_Handler(void)
{
    while (true)
    {
    }
}

//This function handles Memory management fault.
extern "C" void MemManage_Handler(void)
{
    /* USER CODE END MemoryManagement_IRQn 0 */
    while (true)
    {
    }
}

//This function handles Pre-fetch fault, memory access fault.
extern "C" void BusFault_Handler(void)
{
    while (true)
    {
    }
}

//This function handles Undefined instruction or illegal state.
extern "C" void UsageFault_Handler(void)
{
    while (true)
    {
    }
}

//This function handles System service call via SWI instruction.
extern "C" void SVC_Handler(void)
{
}

//This function handles Debug monitor.
extern "C" void DebugMon_Handler(void)
{
}

//This function handles Pendable request for system service.
extern "C" void PendSV_Handler(void)
{
}

//This function handles System tick timer.
extern "C" void SysTick_Handler(void)
{
    HAL_IncTick();

#ifdef FW_CDC
    //use this timer to check for incoming traffic on UART
    static size_t timerCounter = 0;

    if (++timerCounter == CDC_POLLING_TIME)
    {
        timerCounter = 0;
        Board::detail::cdc::checkIncomingData();
    }
#endif
}

#if defined(FW_APP) || defined(FW_CDC)
//not needed in bootloader
#ifdef USE_UART
extern "C" void USART1_IRQHandler(void)
{
    Board::detail::isrHandling::uart(0);
}

extern "C" void USART2_IRQHandler(void)
{
    Board::detail::isrHandling::uart(1);
}

extern "C" void USART3_IRQHandler(void)
{
    Board::detail::isrHandling::uart(2);
}

extern "C" void UART4_IRQHandler(void)
{
    Board::detail::isrHandling::uart(3);
}

extern "C" void UART5_IRQHandler(void)
{
    Board::detail::isrHandling::uart(4);
}

extern "C" void USART6_IRQHandler(void)
{
    Board::detail::isrHandling::uart(5);
}
#endif

#ifdef FW_APP
extern "C" void ADC_IRQHandler(void)
{
    Board::detail::isrHandling::adc(ADC_INSTANCE->DR);
}
#endif
#endif

#ifdef TIM4
extern "C" void TIM4_IRQHandler(void)
{
    TIM4->SR = ~TIM_IT_UPDATE;

    if (TIM4 == MAIN_TIMER_INSTANCE)
    {
        Board::detail::isrHandling::mainTimer();
    }
    else if (TIM4 == PWM_TIMER_INSTANCE)
    {
#ifdef FW_APP
#ifndef USB_LINK_MCU
#if MAX_NUMBER_OF_LEDS > 0
        Board::detail::io::checkDigitalOutputs();
#endif
#endif
#endif
    }
}
#endif

#ifdef TIM5
extern "C" void TIM5_IRQHandler(void)
{
    TIM5->SR = ~TIM_IT_UPDATE;

    if (TIM5 == MAIN_TIMER_INSTANCE)
    {
        Board::detail::isrHandling::mainTimer();
    }
    else if (TIM5 == PWM_TIMER_INSTANCE)
    {
#ifdef FW_APP
#ifndef USB_LINK_MCU
#if MAX_NUMBER_OF_LEDS > 0
        Board::detail::io::checkDigitalOutputs();
#endif
#endif
#endif
    }
}
#endif

#ifdef TIM7
extern "C" void TIM7_IRQHandler(void)
{
    TIM7->SR = ~TIM_IT_UPDATE;

    if (TIM7 == MAIN_TIMER_INSTANCE)
    {
        Board::detail::isrHandling::mainTimer();
    }
    else if (TIM7 == PWM_TIMER_INSTANCE)
    {
#ifdef FW_APP
#ifndef USB_LINK_MCU
#if MAX_NUMBER_OF_LEDS > 0
        Board::detail::io::checkDigitalOutputs();
#endif
#endif
#endif
    }
}
#endif