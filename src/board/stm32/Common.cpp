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
#include "stm32f4xx_hal.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Reset.h"
#include "core/src/general/Interrupt.h"
#include "eeprom/EEPROM.h"

namespace core
{
    namespace timing
    {
        namespace detail
        {
            ///
            /// \brief Implementation of core variable used to keep track of run time in milliseconds.
            ///
            volatile uint32_t rTime_ms;
        }    // namespace detail
    }        // namespace timing
}    // namespace core

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
    while (1)
    {
    }
}

//This function handles Memory management fault.
extern "C" void MemManage_Handler(void)
{
    /* USER CODE END MemoryManagement_IRQn 0 */
    while (1)
    {
    }
}

//This function handles Pre-fetch fault, memory access fault.
extern "C" void BusFault_Handler(void)
{
    while (1)
    {
    }
}

//This function handles Undefined instruction or illegal state.
extern "C" void UsageFault_Handler(void)
{
    while (1)
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
}

namespace
{
    EmuEEPROM emuEEPROM(Board::detail::map::eepromFlashPage1(), Board::detail::map::eepromFlashPage2());
}    // namespace

namespace Board
{
    namespace detail
    {
        namespace isrHandling
        {
            void mainTimer()
            {
                static bool _1ms = true;
                _1ms = !_1ms;

                if (_1ms)
                {
                    core::timing::detail::rTime_ms++;

#ifdef LEDS_SUPPORTED
                    Board::detail::io::checkDigitalOutputs();
#endif
#ifdef LED_INDICATORS
                    Board::detail::io::checkIndicators();
#endif
                }

                Board::detail::io::checkDigitalInputs();
            }
        }    // namespace isrHandling
    }        // namespace detail

    void init()
    {
        //Reset of all peripherals, Initializes the Flash interface and the Systick
        HAL_Init();

        detail::setup::clocks();
        detail::setup::io();
        detail::setup::adc();

        emuEEPROM.init();

        detail::setup::timers();

#ifdef USB_MIDI_SUPPORTED
        detail::setup::usb();
#endif
    }

    void reboot(rebootType_t type)
    {
        core::reset::mcuReset();
    }

    bool checkNewRevision()
    {
        return false;
    }

    void uniqueID(uniqueID_t& uid)
    {
        uint32_t id[3];

        id[0] = HAL_GetUIDw0();
        id[1] = HAL_GetUIDw1();
        id[2] = HAL_GetUIDw2();

        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 4; j++)
                uid.uid[(i * 4) + j] = id[i] >> ((3 - j) * 8) & 0xFF;
        }
    }

    namespace eeprom
    {
        bool read(uint32_t address, LESSDB::sectionParameterType_t type, int32_t& value)
        {
            uint16_t tempData;

            switch (type)
            {
            case LESSDB::sectionParameterType_t::bit:
            case LESSDB::sectionParameterType_t::byte:
            case LESSDB::sectionParameterType_t::halfByte:
            case LESSDB::sectionParameterType_t::word:
                if (emuEEPROM.read(address, tempData) != EmuEEPROM::readStatus_t::ok)
                    return false;
                else
                    value = tempData;
                break;

            default:
                return false;
                break;
            }

            return true;
        }

        bool write(uint32_t address, int32_t value, LESSDB::sectionParameterType_t type)
        {
            uint16_t tempData;

            switch (type)
            {
            case LESSDB::sectionParameterType_t::bit:
            case LESSDB::sectionParameterType_t::byte:
            case LESSDB::sectionParameterType_t::halfByte:
            case LESSDB::sectionParameterType_t::word:
                tempData = value;
                if (emuEEPROM.write(address, tempData) != EmuEEPROM::writeStatus_t::ok)
                    return false;
                break;

            default:
                return false;
                break;
            }

            return true;
        }
    }    // namespace eeprom
}    // namespace Board