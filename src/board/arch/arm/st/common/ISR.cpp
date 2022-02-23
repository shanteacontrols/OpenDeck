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
#include "core/src/general/Timing.h"
#include <MCU.h>

// STM32 common ISRs

// This function handles Non maskable interrupt.
extern "C" void NMI_Handler(void)
{
}

// This function handles Memory management fault.
extern "C" void MemManage_Handler(void)
{
    /* USER CODE END MemoryManagement_IRQn 0 */
    while (true)
    {
    }
}

// This function handles Pre-fetch fault, memory access fault.
extern "C" void BusFault_Handler(void)
{
    while (true)
    {
    }
}

// This function handles Undefined instruction or illegal state.
extern "C" void UsageFault_Handler(void)
{
    while (true)
    {
    }
}

// This function handles System service call via SWI instruction.
extern "C" void SVC_Handler(void)
{
}

// This function handles Debug monitor.
extern "C" void DebugMon_Handler(void)
{
}

// This function handles Pendable request for system service.
extern "C" void PendSV_Handler(void)
{
}

// This function handles System tick timer.
extern "C" void SysTick_Handler(void)
{
    HAL_IncTick();
}