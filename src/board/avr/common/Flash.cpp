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

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/boot.h>
#include "board/Board.h"
#include "board/Internal.h"
#include "core/src/arch/avr/Misc.h"

namespace Board
{
    namespace detail
    {
        namespace flash
        {
            bool isInRange(uint32_t address)
            {
                return address <= FLASHEND;
            }

            uint32_t size()
            {
                return FLASHEND + static_cast<uint32_t>(1);
            }

            uint32_t pageSize(size_t index)
            {
                //always constant on avr
                return SPM_PAGESIZE;
            }

            bool erasePage(size_t index)
            {
                boot_page_erase(index * pageSize(index));
                boot_spm_busy_wait();

                return true;
            }

            void writePage(size_t index)
            {
                //write the filled flash page to memory
                boot_page_write(index * pageSize(index));
                boot_spm_busy_wait();

                //re-enable RWW section
                boot_rww_enable();
            }

            bool write16(uint32_t address, uint16_t data)
            {
                boot_page_fill(address, data);
                return true;
            }

            bool read8(uint32_t address, uint8_t& data)
            {
                data = pgm_read_byte(address);
                return true;
            }

            bool read16(uint32_t address, uint16_t& data)
            {
                data = pgm_read_word(address);
                return true;
            }

            bool read32(uint32_t address, uint32_t& data)
            {
                data = pgm_read_dword(address);
                return true;
            }
        }    // namespace flash
    }        // namespace detail
}    // namespace Board