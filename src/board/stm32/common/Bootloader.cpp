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
#include "board/common/constants/Reboot.h"
#include "core/src/general/Interrupt.h"
#include "stm32f4xx_hal.h"

using appEntry_t = void (*)();

namespace
{
    ///
    /// \brief Variable used to specify whether to enter bootloader or application.
    ///
    uint32_t fwEntryType __attribute__((section(".noinit"))) __attribute__((used));
}    // namespace

namespace Board
{
    namespace bootloader
    {
        uint32_t pageSize(size_t index)
        {
            return detail::map::flashPageDescriptor(index).size;
        }

        void erasePage(size_t index)
        {
        }

        void fillPage(size_t index, uint32_t address, uint16_t data)
        {
        }

        void writePage(size_t index)
        {
        }

        void applyFw()
        {
        }
    }    // namespace bootloader

    namespace detail
    {
        namespace bootloader
        {
            bool isAppValid()
            {
                return true;
            }

            bool isSWtriggerActive()
            {
                return fwEntryType == BTLDR_REBOOT_VALUE;
            }

            void enableSWtrigger()
            {
                fwEntryType = BTLDR_REBOOT_VALUE;
            }

            void clearSWtrigger()
            {
                fwEntryType = APP_REBOOT_VALUE;
            }

            void runApplication()
            {
                HAL_RCC_DeInit();
                HAL_DeInit();

                appEntry_t appEntry = (appEntry_t) * (volatile uint32_t*)(APP_START_ADDR + 4);
                appEntry();

                while (1)
                    ;
            }

            void runBootloader()
            {
                detail::bootloader::indicate();

#ifdef USB_MIDI_SUPPORTED
                detail::setup::usb();
#endif
            }
        }    // namespace bootloader
    }        // namespace detail
}    // namespace Board