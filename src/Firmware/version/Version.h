/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2017 Igor Petrovic

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
#define HARDWARE_VERSION_REVISION   1
/// @}

///
/// \brief Location at which firmware version is written in compiled binary.
/// 6 bytes are used in total (two for each software version point).
///
#define SW_VERSION_POINT_LOCATION  (FLASH_SIZE - 8)

///
/// \brief Location at which CRC of compiled binary is written in compiled binary.
///
#define SW_CRC_LOCATION_FLASH      (FLASH_SIZE - 2)

///
/// \brief Location at which compiled binary CRC is written in EEPROM.
///
#define SW_CRC_LOCATION_EEPROM     (EEPROM_SIZE - 3)

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

/// @}