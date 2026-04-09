/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inttypes.h>

/** @brief Returns the firmware version. */
constexpr inline uint8_t SYSEX_CR_FIRMWARE_VERSION = 0x56;
/** @brief Returns the hardware UID. */
constexpr inline uint8_t SYSEX_CR_HARDWARE_UID = 0x42;
/** @brief Returns the firmware version together with the hardware UID. */
constexpr inline uint8_t SYSEX_CR_FIRMWARE_VERSION_HARDWARE_UID = 0x43;
/** @brief Requests an application reboot. */
constexpr inline uint8_t SYSEX_CR_REBOOT_APP = 0x7F;
/** @brief Requests a bootloader reboot. */
constexpr inline uint8_t SYSEX_CR_REBOOT_BTLDR = 0x55;
/** @brief Requests a factory reset. */
constexpr inline uint8_t SYSEX_CR_FACTORY_RESET = 0x44;
/** @brief Returns the maximum supported component count. */
constexpr inline uint8_t SYSEX_CR_MAX_COMPONENTS = 0x4D;
/** @brief Returns the number of supported presets. */
constexpr inline uint8_t SYSEX_CR_SUPPORTED_PRESETS = 0x50;
/** @brief Returns whether bootloader support is available. */
constexpr inline uint8_t SYSEX_CR_BOOTLOADER_SUPPORT = 0x51;
/** @brief Starts a full-backup transfer. */
constexpr inline uint8_t SYSEX_CR_FULL_BACKUP = 0x1B;
/** @brief Starts a restore transfer. */
constexpr inline uint8_t SYSEX_CR_RESTORE_START = 0x1C;
/** @brief Ends a restore transfer. */
constexpr inline uint8_t SYSEX_CR_RESTORE_END = 0x1D;

/** @brief Identifies a component-info message sent to the host. */
constexpr inline uint8_t SYSEX_CM_COMPONENT_ID = 0x49;
