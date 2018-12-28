/*

Copyright 2015-2018 Igor Petrovic

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

#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include "board/Board.h"
#include "../usb/USB_MIDI.h"
#include "Common.h"
#include "board/common/constants/Reboot.h"
#include "board/common/uart/Variables.h"
#include "board/common/uart/ODformat.h"
#include "midi/src/Constants.h"
#include "core/src/HAL/avr/reset/Reset.h"
#include "core/src/HAL/avr/Misc.h"
#include "core/src/general/Misc.h"
#include "Init.cpp"

namespace Board
{
    using namespace Board::detail;

    namespace
    {
        ///
        /// \brief Placeholder variable used only to reserve space in linker section.
        ///
        const uint32_t appLength __attribute__ ((section (".applen"))) __attribute__((used)) = 0;
    }

    void init()
    {
        cli();
        //disable watchdog
        MCUSR &= ~(1 << WDRF);
        wdt_disable();

        ///
        /// \brief Initializes all pins to correct states.
        ///
        initPins();

        #ifndef BOARD_A_xu2
        ///
        /// \brief Initializes analog variables and ADC peripheral.
        ///
        initAnalog();
        #endif

        #ifdef USB_SUPPORTED
        ///
        /// \brief Initializes USB peripheral and configures it as MIDI device.
        ///
        initMIDI_USB();
        #endif

        ///
        /// \brief Initializes main and PWM timers.
        ///
        configureTimers();
    }

    void reboot(rebootType_t type)
    {
        switch(type)
        {
            case rebootType_t::rebootApp:
            eeprom_write_byte(reinterpret_cast<uint8_t*>(REBOOT_VALUE_EEPROM_LOCATION), APP_REBOOT_VALUE);
            break;

            case rebootType_t::rebootBtldr:
            eeprom_write_byte(reinterpret_cast<uint8_t*>(REBOOT_VALUE_EEPROM_LOCATION), BTLDR_REBOOT_VALUE);
            break;
        }

        mcuReset();
    }

    #ifndef BOARD_A_xu2

    bool checkNewRevision()
    {
        uint16_t crc_eeprom = eeprom_read_word(reinterpret_cast<uint16_t*>(SW_CRC_LOCATION_EEPROM));
        #if (FLASHEND > 0xFFFF)
        uint32_t lastAddress = pgm_read_dword_far(pgmGetFarAddress(APP_LENGTH_LOCATION));
        uint16_t crc_flash = pgm_read_word_far(pgmGetFarAddress(lastAddress));
        #else
        uint32_t lastAddress = pgm_read_dword(APP_LENGTH_LOCATION);
        uint16_t crc_flash = pgm_read_word(lastAddress);
        #endif

        if (crc_eeprom != crc_flash)
        {
            eeprom_update_word(reinterpret_cast<uint16_t*>(SW_CRC_LOCATION_EEPROM), crc_flash);
            return true;
        }

        return false;
    }

    uint16_t scaleADC(uint16_t value, uint16_t maxValue)
    {
        if (maxValue == ADC_MAX_VALUE)
        {
            return value;
        }
        else if (maxValue == MIDI_7_BIT_VALUE_MAX)
        {
            return value >> 3;
        }
        else if (maxValue == MIDI_14_BIT_VALUE_MAX)
        {
            return value << 4;
        }
        else
        {
            //use mapRange_uint32 to avoid overflow issues
            return mapRange_uint32(value, 0, ADC_MAX_VALUE, 0, maxValue);
        }
    }

    bool memoryRead(uint32_t address, sectionParameterType_t type, int32_t &value)
    {
        switch(type)
        {
            case BIT_PARAMETER:
            case BYTE_PARAMETER:
            case HALFBYTE_PARAMETER:
            value = eeprom_read_byte(reinterpret_cast<uint8_t*>(address));
            break;

            case WORD_PARAMETER:
            value = eeprom_read_word(reinterpret_cast<uint16_t*>(address));
            break;

            default:
            // case DWORD_PARAMETER:
            value = eeprom_read_dword(reinterpret_cast<uint32_t*>(address));
            break;
        }

        return true;
    }

    bool memoryWrite(uint32_t address, int32_t value, sectionParameterType_t type)
    {
        switch(type)
        {
            case BIT_PARAMETER:
            case BYTE_PARAMETER:
            case HALFBYTE_PARAMETER:
            eeprom_update_byte(reinterpret_cast<uint8_t*>(address), value);
            break;

            case WORD_PARAMETER:
            eeprom_update_word(reinterpret_cast<uint16_t*>(address), value);
            break;

            default:
            // case DWORD_PARAMETER:
            eeprom_update_dword(reinterpret_cast<uint32_t*>(address), value);
            break;
        }

        return true;
    }

    #endif
}