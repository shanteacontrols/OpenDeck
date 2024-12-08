/*

Copyright Igor Petrovic

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

#include <inttypes.h>

/// Custom requests for SysEx protocol

constexpr inline uint8_t SYSEX_CR_FIRMWARE_VERSION              = 0x56;
constexpr inline uint8_t SYSEX_CR_HARDWARE_UID                  = 0x42;
constexpr inline uint8_t SYSEX_CR_FIRMWARE_VERSION_HARDWARE_UID = 0x43;
constexpr inline uint8_t SYSEX_CR_REBOOT_APP                    = 0x7F;
constexpr inline uint8_t SYSEX_CR_REBOOT_BTLDR                  = 0x55;
constexpr inline uint8_t SYSEX_CR_FACTORY_RESET                 = 0x44;
constexpr inline uint8_t SYSEX_CR_MAX_COMPONENTS                = 0x4D;
constexpr inline uint8_t SYSEX_CR_SUPPORTED_PRESETS             = 0x50;
constexpr inline uint8_t SYSEX_CR_BOOTLOADER_SUPPORT            = 0x51;
constexpr inline uint8_t SYSEX_CR_FULL_BACKUP                   = 0x1B;
constexpr inline uint8_t SYSEX_CR_RESTORE_START                 = 0x1C;
constexpr inline uint8_t SYSEX_CR_RESTORE_END                   = 0x1D;

/// Custom ID used when sending info about components to host
constexpr inline uint8_t SYSEX_CM_COMPONENT_ID = 0x49;