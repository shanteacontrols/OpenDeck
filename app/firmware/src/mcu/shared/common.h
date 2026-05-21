/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace opendeck::mcu
{
    /**
     * @brief Boot targets the firmware can request from the platform boot chain.
     */
    enum class BootTarget : uint8_t
    {
        Application,
        Bootloader,
    };

    /**
     * @brief Maximum number of hardware serial-number bytes exposed by firmware APIs.
     */
    constexpr inline size_t SERIAL_NUMBER_BUFFER_SIZE = 16;
}    // namespace opendeck::mcu
