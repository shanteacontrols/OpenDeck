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
            //normally, on avr, flash page size is constant for all page sizes
            //and it's defined as SPM_PAGESIZE
            //in the case of arduino mega board, USB link MCU is atmega16u2 and main MCU is atmega2560
            //using SPM_PAGESIZE would use page size for 16u2 which is 128 bytes, when, in fact,
            //atmega2560 is being flashed via atmega16u2
            //therefore, in that case use atmega2560 page size which is 256 bytes
            //on arduino uno, similar setup is used (atmega16u2 acts as USB link to main MCU which is atmega328p)
            //however, both MCUs have same SPM_PAGESIZE which is 128
#ifdef OD_BOARD_MEGA16U2
            return 256;
#else
            return SPM_PAGESIZE;
#endif
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