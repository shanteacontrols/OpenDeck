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
}    // namespace fw_selector
