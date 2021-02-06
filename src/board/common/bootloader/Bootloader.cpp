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
#include "board/common/io/Helpers.h"
#include "Pins.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Reset.h"
#include "core/src/general/CRC.h"
#include "MCU.h"

namespace Board
{
    namespace bootloader
    {
        void appAddrBoundary(uint32_t& first, uint32_t& last)
        {
            first = APP_START_ADDR;
            detail::flash::read32(FW_METADATA_LOCATION, last);
        }

        bool isHWtriggerActive()
        {
            //add some delay before reading the pins to avoid incorrect state detection
            core::timing::waitMs(100);

#if defined(BTLDR_BUTTON_PORT)
#ifdef BTLDR_BUTTON_AH
            return CORE_IO_READ(BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN);
#else
            return !CORE_IO_READ(BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN);
#endif
#else
            //no hardware entry possible in this case
            return false;
#endif
        }

        uint32_t pageSize(size_t index)
        {
            return detail::flash::pageSize(index + BOOTLOADER_PAGE_START_INDEX);
        }

        void erasePage(size_t index)
        {
            detail::flash::erasePage(index + BOOTLOADER_PAGE_START_INDEX);
        }

        void fillPage(size_t index, uint32_t address, uint16_t data)
        {
            detail::flash::write16(detail::map::flashPageDescriptor(index + BOOTLOADER_PAGE_START_INDEX).address + address, data);
        }

        void writePage(size_t index)
        {
            detail::flash::writePage(index + BOOTLOADER_PAGE_START_INDEX);
        }

        uint8_t readFlash(uint32_t address)
        {
            uint8_t data = 0;

            if (!detail::flash::read8(address, data))
                data = 0;

            return data;
        }
    }    // namespace bootloader
}    // namespace Board