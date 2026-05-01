/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

namespace fw_selector
{
    /**
     * @brief Identifies which firmware image should be started.
     */
    enum class FwType : uint32_t
    {
        Application = 0xFFFFFFFF,
        Bootloader  = 0x47474747
    };

    /**
     * @brief Identifies why a particular firmware image was selected.
     */
    enum class Trigger : uint8_t
    {
        Software,
        Hardware,
        InvalidApp,
        None
    };

    /**
     * @brief Describes the firmware-selection outcome.
     */
    struct Selection
    {
        FwType  firmware = FwType::Bootloader;
        Trigger trigger  = Trigger::None;
    };

    /**
     * @brief Half-open memory address range.
     */
    struct Boundary
    {
        uint32_t start = 0;
        uint32_t end   = 0;
    };

    /**
     * @brief Address layout needed to validate and start the application.
     */
    struct AppInfo
    {
        Boundary app                 = {};
        Boundary sram                = {};
        uint32_t vector_table_offset = 0;
    };

    /**
     * @brief Magic value that marks the installed-image validation record.
     */
    constexpr inline uint32_t IMAGE_VALIDATION_MAGIC = 0x43554644U;

    /**
     * @brief Number of bytes in the installed-image validation record.
     */
    constexpr inline uint32_t IMAGE_VALIDATION_RECORD_SIZE = sizeof(uint32_t) * 3U;
}    // namespace fw_selector
