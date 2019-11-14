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

#pragma once

///
/// \brief Common defines for all variants based on this MCU.
/// @{

///
/// \brief Defines the size of the single flash sector of emulated EEPROM.
///
#define EEPROM_PAGE_SIZE            (uint32_t)0x20000

///
/// \brief Address at which first flash sector dedicated to emulated EEPROM starts.
///
#define EEPROM_START_ADDRESS        ((uint32_t)FLASH_BASE + (uint32_t)0x20000)

#define EEPROM_PAGE1_START_ADDRESS  EEPROM_START_ADDRESS
#define EEPROM_PAGE2_START_ADDRESS  (EEPROM_PAGE1_START_ADDRESS + EEPROM_PAGE_SIZE)

#define EEPROM_PAGE1_SECTOR         FLASH_SECTOR_5
#define EEPROM_PAGE2_SECTOR         FLASH_SECTOR_6

///
/// \brief Total available words (16-bit) for data in EEPROM.
/// Emulated EEPROM uses the following layout:
/// Header/status (2 bytes)
/// 2 bytes of single value
/// 2 bytes for value address
///
#define EEPROM_SIZE                 (EEPROM_PAGE_SIZE / 8)

///
/// \brief MCU voltage needed in order to write words to flash memory (2.7V to 3.6V).
///
#define EEPROM_VOLTAGE_RANGE        (uint8_t)FLASH_VOLTAGE_RANGE_3

/// @}