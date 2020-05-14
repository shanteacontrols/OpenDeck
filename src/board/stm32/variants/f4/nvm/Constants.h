#pragma once

///
/// \brief Defines the size of the single flash sector of emulated EEPROM.
///
#define EEPROM_PAGE_SIZE (uint32_t)0x20000

///
/// \brief Address at which first flash sector dedicated to emulated EEPROM starts.
///
#define EEPROM_START_ADDRESS ((uint32_t)FLASH_BASE + (uint32_t)0x20000)

#define EEPROM_PAGE1_START_ADDRESS EEPROM_START_ADDRESS
#define EEPROM_PAGE2_START_ADDRESS (EEPROM_PAGE1_START_ADDRESS + EEPROM_PAGE_SIZE)

#define EEPROM_PAGE1_SECTOR FLASH_SECTOR_5
#define EEPROM_PAGE2_SECTOR FLASH_SECTOR_6

///
/// \brief Total available bytes for data in EEPROM.
///
#define EEPROM_SIZE EEPROM_PAGE_SIZE

///
/// \brief MCU voltage needed in order to write words to flash memory (2.7V to 3.6V).
///
#define EEPROM_VOLTAGE_RANGE (uint8_t) FLASH_VOLTAGE_RANGE_3