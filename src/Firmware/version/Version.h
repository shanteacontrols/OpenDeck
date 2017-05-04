#pragma once

#include "DataTypes.h"

///
/// \brief Firmware and hardware versioning.
/// \addtogroup version
/// @{

///
/// \brief Hardcoded board revision.
/// @{
///
#define HARDWARE_VERSION_MAJOR      1
#define HARDWARE_VERSION_MINOR      1
#define HARDWARE_VERSION_REVISION   0
/// @}

///
/// \brief Location at which firmware version is written in compiled binary.
/// 6 bytes are used in total (two for each software version point).
/// 7 bytes are used for Git hash.
/// 2 bytes are used for CRC value.
///
#define SW_VERSION_POINT_LOCATION  (FLASH_SIZE - 6 - 7 - 2)

///
/// \brief Location at which CRC of compiled binary is written in compiled binary.
///
#define SW_CRC_LOCATION_FLASH      (FLASH_SIZE - 2)

///
/// \brief Location at which compiled binary CRC is written in EEPROM.
///
#define SW_CRC_LOCATION_EEPROM     (EEPROM_SIZE - 4)

///
/// \brief Number of bytes for short Git hash.
/// Git hash has 7 bytes, add 1 for EOL char.
///
#define GIT_HASH_BYTES              7+1

///
/// \brief Checks if firmware has been updated.
/// Firmware file has written CRC in last two flash addresses. Application stores last read CRC in EEPROM. If EEPROM and flash CRC differ, firmware has been updated.
///
bool checkNewRevision();

///
/// \brief Returns software version.
/// @param [in] point   Software version point (swVersion_major, swVersion_minor or swVersion_revision).
/// \return Software version point.
///
uint8_t getSWversion(swVersion_t point);

///
/// \brief Reads Git hash stored in flash.
/// @param [in,out] hash    Char array in which to store hash.
///
void getGitHash(char *hash);

/// @}