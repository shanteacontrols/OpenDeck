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

#include <avr/boot.h>
#include "board/Board.h"
#include "board/Internal.h"
#include "core/src/general/Reset.h"
#include "core/src/general/Helpers.h"

namespace Board
{
#ifdef FW_BOOT
    namespace bootloader
    {
        size_t pageSize(size_t index)
        {
            return BTLDR_FLASH_PAGE_SIZE;
        }

        void erasePage(uint32_t address)
        {
            boot_page_erase(address);
            boot_spm_busy_wait();
        }

        void fillPage(uint32_t address, uint16_t data)
        {
            boot_page_fill(address, data);
        }

        void writePage(uint32_t address)
        {
            //write the filled FLASH page to memory
            boot_page_write(address);
            boot_spm_busy_wait();

            //re-enable RWW section
            boot_rww_enable();
        }

        void applyFw()
        {
            core::reset::mcuReset();
        }
    }    // namespace bootloader
#endif

    namespace detail
    {
        namespace bootloader
        {
            bool isSWtriggerActive()
            {
                return eeprom_read_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION) == BTLDR_REBOOT_VALUE;
            }

            void enableSWtrigger()
            {
                eeprom_write_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION, BTLDR_REBOOT_VALUE);
            }

            void clearSWtrigger()
            {
                eeprom_write_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION, APP_REBOOT_VALUE);
            }
        }    // namespace bootloader
    }        // namespace detail
}    // namespace Board