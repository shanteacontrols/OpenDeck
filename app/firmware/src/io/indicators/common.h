/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

namespace io::indicators
{
    /**
     * @brief Identifies an indicator output or a group operation target.
     */
    enum class Type : uint8_t
    {
        UsbIn,
        UsbOut,
        DinIn,
        DinOut,
        BleIn,
        BleOut,
        All,
    };
}    // namespace io::indicators
