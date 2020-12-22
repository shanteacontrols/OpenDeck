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
#include "core/src/general/Interrupt.h"

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
    namespace detail
    {
        namespace bootloader
        {
            fwType_t btldrTriggerSoftType()
            {
                if (fwEntryType == static_cast<uint8_t>(fwType_t::bootloader))
                    return fwType_t::bootloader;
                else if (fwEntryType == static_cast<uint8_t>(fwType_t::cdc))
                    return fwType_t::cdc;
                else
                    return fwType_t::application;
            }

            void setSWtrigger(fwType_t btldrTriggerSoftType)
            {
                fwEntryType = static_cast<uint8_t>(btldrTriggerSoftType);
            }

            void runApplication()
            {
                HAL_RCC_DeInit();
                HAL_DeInit();

                auto appEntry = (appEntry_t) * (volatile uint32_t*)(APP_START_ADDR + 4);
                appEntry();

                while (true)
                    ;
            }

            void runCDC()
            {
                HAL_RCC_DeInit();
                HAL_DeInit();

                auto appEntry = (appEntry_t) * (volatile uint32_t*)(CDC_START_ADDR + 4);
                appEntry();

                while (true)
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