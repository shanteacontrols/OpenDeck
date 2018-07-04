/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "board/Board.h"
#include "Common.h"
#include "board/common/constants/Reboot.h"

///
/// \brief Placeholder variable used only to reserve space in linker section.
///
const uint32_t appLength __attribute__ ((section (".applen"))) __attribute__((used)) = 0;

void Board::init()
{
    cli();
    //disable watchdog
    MCUSR &= ~(1 << WDRF);
    wdt_disable();
    initPins();
    initAnalog();
    initEncoders();

    #ifdef DIN_MIDI_SUPPORTED
    initMIDI_UART();
    #endif

    #ifdef USB_SUPPORTED
    initMIDI_USB();
    #endif

    configureTimers();
    initCustom();
}

bool Board::checkNewRevision()
{
    #if !defined(BOARD_A_MEGA) && !defined(BOARD_A_UNO)
    uint32_t flash_size = pgm_read_dword(APP_LENGTH_LOCATION);
    uint16_t crc_eeprom = eeprom_read_word((uint16_t*)SW_CRC_LOCATION_EEPROM);
    uint16_t crc_flash = pgm_read_word(flash_size);

    if (crc_eeprom != crc_flash)
    {
        eeprom_update_word((uint16_t*)SW_CRC_LOCATION_EEPROM, crc_flash);
        return true;
    }
    #endif

    return false;
}

void Board::reboot(rebootType_t type)
{
    switch(type)
    {
        case rebootApp:
        eeprom_write_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION, APP_REBOOT_VALUE);
        break;

        case rebootBtldr:
        eeprom_write_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION, BTLDR_REBOOT_VALUE);
        break;
    }

    mcuReset();
}

uint16_t Board::scaleADC(uint16_t value, uint16_t maxValue)
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

bool Board::memoryRead(uint32_t address, sectionParameterType_t type, int32_t &value)
{
    switch(type)
    {
        case BIT_PARAMETER:
        case BYTE_PARAMETER:
        case HALFBYTE_PARAMETER:
        value = eeprom_read_byte((uint8_t*)address);
        break;

        case WORD_PARAMETER:
        value = eeprom_read_word((uint16_t*)address);
        break;

        default:
        // case DWORD_PARAMETER:
        value = eeprom_read_dword((uint32_t*)address);
        break;
    }

    return true;
}

bool Board::memoryWrite(uint32_t address, int32_t value, sectionParameterType_t type)
{
    switch(type)
    {
        case BIT_PARAMETER:
        case BYTE_PARAMETER:
        case HALFBYTE_PARAMETER:
        eeprom_update_byte((uint8_t*)address, value);
        break;

        case WORD_PARAMETER:
        eeprom_update_word((uint16_t*)address, value);
        break;

        default:
        // case DWORD_PARAMETER:
        eeprom_update_dword((uint32_t*)address, value);
        break;
    }

    return true;
}