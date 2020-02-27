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

#include <avr/boot.h>
#include "board/Board.h"
#include "board/Internal.h"
#include "board/common/constants/Bootloader.h"
#include "bootloader/mcu/Config.h"
#include "core/src/general/Reset.h"
#include "core/src/general/Helpers.h"

namespace Board
{
#ifdef FW_BOOT
    namespace bootloader
    {
        void checkPackets()
        {
#ifndef USB_MIDI_SUPPORTED
            static bool    btldrInProgress;
            static uint8_t byteCount;
            static uint8_t lower;
            static uint8_t upper;
            uint8_t        data;

            if (Board::UART::read(UART_USB_LINK_CHANNEL, data))
            {
                if (!btldrInProgress)
                {
                    if (data == COMMAND_START_FLASHING_UART)
                        btldrInProgress = true;
                }
                else
                {
                    if (!byteCount)
                    {
                        lower = data;
                        byteCount++;
                    }
                    else
                    {
                        byteCount = 0;
                        upper     = data;

                        uint16_t word = GET_WORD(lower, upper);
                        packetHandler(word);
                    }
                }
            }
#else
            //nothing, lufa will fire interrupt once packet from usb is received
#endif
        }

        void erasePage(uint32_t address)
        {
            //don't do anything on USB link MCU
#ifndef USB_LINK_MCU
            boot_page_erase(address);
            boot_spm_busy_wait();
#endif
        }

        void fillPage(uint32_t address, uint16_t data)
        {
#ifndef USB_LINK_MCU
            boot_page_fill(address, data);
#else
            //mark the start of bootloader packets
            //used to avoid possible junk coming on main mcu uart as first byte
            if (!address)
                Board::UART::write(UART_USB_LINK_CHANNEL, COMMAND_START_FLASHING_UART);

            //on USB link MCU, forward the received flash via UART to main MCU
            //send address only when necessary (address first, BTLDR_FLASH_PAGE_SIZE bytes next)

            if (!(address % BTLDR_FLASH_PAGE_SIZE))
            {
                uint16_t lowAddress  = address & 0xFFFF;
                uint16_t highAddress = address >> 16;

                Board::UART::write(UART_USB_LINK_CHANNEL, LSB_WORD(lowAddress));
                Board::UART::write(UART_USB_LINK_CHANNEL, MSB_WORD(lowAddress));

                Board::UART::write(UART_USB_LINK_CHANNEL, LSB_WORD(highAddress));
                Board::UART::write(UART_USB_LINK_CHANNEL, MSB_WORD(highAddress));
            }

            Board::UART::write(UART_USB_LINK_CHANNEL, LSB_WORD(data));
            Board::UART::write(UART_USB_LINK_CHANNEL, MSB_WORD(data));
#endif
        }

        void writePage(uint32_t address)
        {
            //don't do anything on USB link MCU
#ifndef USB_LINK_MCU
            //write the filled FLASH page to memory
            boot_page_write(address);
            boot_spm_busy_wait();

            //re-enable RWW section
            boot_rww_enable();
#endif
        }

        void applyFw()
        {
#ifdef USB_LINK_MCU
            //make sure USB link sends the "start application" instruction to the main MCU

            uint16_t lowAddress  = COMMAND_STARTAPPLICATION & 0xFFFF;
            uint16_t highAddress = COMMAND_STARTAPPLICATION >> 16;

            Board::UART::write(UART_USB_LINK_CHANNEL, LSB_WORD(lowAddress));
            Board::UART::write(UART_USB_LINK_CHANNEL, MSB_WORD(lowAddress));

            Board::UART::write(UART_USB_LINK_CHANNEL, LSB_WORD(highAddress));
            Board::UART::write(UART_USB_LINK_CHANNEL, MSB_WORD(highAddress));

            while (!Board::UART::isTxEmpty(UART_USB_LINK_CHANNEL))
                ;
#endif

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