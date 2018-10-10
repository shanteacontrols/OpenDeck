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

#pragma once

///
/// \brief Custom requests for SysEx protocol.
/// @{

#define SYSEX_CR_FIRMWARE_VERSION           0x56
#define SYSEX_CR_HARDWARE_VERSION           0x42
#define SYSEX_CR_FIRMWARE_HARDWARE_VERSION  0x43
#define SYSEX_CR_REBOOT_APP                 0x7F
#define SYSEX_CR_REBOOT_BTLDR               0x55
#define SYSEX_CR_FACTORY_RESET              0x44
#define SYSEX_CR_MAX_COMPONENTS             0x4D
#define SYSEX_CR_ENABLE_PROCESSING          0x65
#define SYSEX_CR_DISABLE_PROCESSING         0x64

/// @}

///
/// \brief Total number of custom requests.
///
#define NUMBER_OF_CUSTOM_REQUESTS           9

///
/// \brief Custom ID used when sending info about components to host.
///
#define SYSEX_CM_COMPONENT_ID               0x49

///
/// \brief Minimum time difference in milliseconds between sending two identical component info messages.
///
#define COMPONENT_INFO_TIMEOUT              500 //ms